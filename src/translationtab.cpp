#include "translationtab.h"

#include "openai_chat_client.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

TranslationTab::TranslationTab(OpenAIChatClient* client, QWidget* parent)
    : QWidget(parent)
    , m_client(client)
    , m_config(defaultConfig())
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(8);

    auto* title = new QLabel(QStringLiteral("文本翻译工具"), this);
    title->setProperty("class", "title");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* langLayout = new QHBoxLayout();
    auto* langLabel = new QLabel(QStringLiteral("翻译方向:"), this);
    m_langCombo = new QComboBox(this);
    m_langCombo->addItem(QStringLiteral("自动检测"), QStringLiteral("auto"));
    m_langCombo->addItem(QStringLiteral("英文 → 中文"), QStringLiteral("en2cn"));
    m_langCombo->addItem(QStringLiteral("中文 → 英文"), QStringLiteral("cn2en"));
    m_langCombo->addItem(QStringLiteral("英文 → 日文"), QStringLiteral("en2jp"));
    m_langCombo->addItem(QStringLiteral("日文 → 中文"), QStringLiteral("jp2cn"));
    m_langCombo->setCurrentIndex(0);
    langLayout->addWidget(langLabel);
    langLayout->addWidget(m_langCombo);
    langLayout->addStretch();
    layout->addLayout(langLayout);

    auto* inputGroup = new QGroupBox(QStringLiteral("输入文本"), this);
    auto* inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(8);
    m_input = new QTextEdit(this);
    m_input->setPlaceholderText(QStringLiteral("请输入要翻译的内容..."));
    inputLayout->addWidget(m_input);
    layout->addWidget(inputGroup);

    auto* outputGroup = new QGroupBox(QStringLiteral("翻译结果"), this);
    auto* outputLayout = new QVBoxLayout(outputGroup);
    outputLayout->setContentsMargins(0, 0, 0, 0);
    outputLayout->setSpacing(8);
    m_output = new QTextEdit(this);
    m_output->setPlaceholderText(QStringLiteral("翻译结果将显示在这里..."));
    outputLayout->addWidget(m_output);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    m_translateButton = new QPushButton(QStringLiteral("开始翻译"), this);
    m_translateButton->setObjectName(QStringLiteral("btn_translate"));
    auto* copyBtn = new QPushButton(QStringLiteral("复制到剪贴板"), this);
    btnLayout->addWidget(m_translateButton, 1);
    btnLayout->addWidget(copyBtn, 1);
    outputLayout->addLayout(btnLayout);

    layout->addWidget(outputGroup);

    connect(m_translateButton, &QPushButton::clicked, this, &TranslationTab::translateText);
    connect(copyBtn, &QPushButton::clicked, this, &TranslationTab::copyTranslation);
}

void TranslationTab::setConfig(const AppConfig& cfg)
{
    m_config = cfg;
}

QString TranslationTab::translationPrompt() const
{
    const auto option = m_langCombo->currentData().toString();
    if (option == QStringLiteral("auto")) {
        return m_config.translation.promptAuto;
    }
    const auto it = m_config.translation.promptMap.find(option);
    if (it != m_config.translation.promptMap.end()) {
        return it.value();
    }
    return m_config.translation.promptAuto;
}

void TranslationTab::translateText()
{
    const auto text = m_input->toPlainText().trimmed();
    if (text.isEmpty()) {
        m_output->setText(QStringLiteral("请输入要翻译的内容"));
        return;
    }

    if (!m_client) {
        m_output->setText(QStringLiteral("网络模块未初始化"));
        return;
    }

    m_translateButton->setEnabled(false);

    const auto prompt = translationPrompt();
    const auto& cfg = m_config.translation;

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), cfg.model);
    payload.insert(QStringLiteral("messages"),
                   QJsonArray{QJsonObject{{QStringLiteral("role"), QStringLiteral("system")},
                                          {QStringLiteral("content"), prompt}},
                              QJsonObject{{QStringLiteral("role"), QStringLiteral("user")},
                                          {QStringLiteral("content"), text}}});
    payload.insert(QStringLiteral("max_tokens"), 2000);

    m_output->setText(QStringLiteral("正在翻译中，请稍候..."));
    QApplication::processEvents();

    m_client->postChatCompletions(
        QUrl(cfg.baseUrl),
        cfg.apiKey,
        payload,
        30000,
        this,
        [this](QString content) {
            m_output->setPlainText(content.trimmed());
            m_translateButton->setEnabled(true);
        },
        [this](QString error) {
            m_output->setText(QStringLiteral("翻译失败: %1").arg(error));
            m_translateButton->setEnabled(true);
        });
}

void TranslationTab::copyTranslation()
{
    QApplication::clipboard()->setText(m_output->toPlainText());
}
