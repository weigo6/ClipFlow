#include "configdialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

ConfigDialog::ConfigDialog(const AppConfig& config, QWidget* parent)
    : QDialog(parent)
    , m_initial(config)
{
    setWindowTitle(QStringLiteral("配置设置"));
    setMinimumSize(500, 400);

    auto* layout = new QVBoxLayout(this);

    auto* tabs = new QTabWidget(this);
    layout->addWidget(tabs);

    auto* imageTab = new QWidget(this);
    auto* imageForm = new QFormLayout(imageTab);
    m_ocrApiKey = new QLineEdit(config.imageToMarkdown.apiKey, this);
    m_ocrApiKey->setPlaceholderText(QStringLiteral("输入OpenAI API密钥"));
    m_ocrModel = new QLineEdit(config.imageToMarkdown.model, this);
    m_ocrModel->setPlaceholderText(QStringLiteral("例如: gpt-4-vision-preview"));
    m_ocrBaseUrl = new QLineEdit(config.imageToMarkdown.baseUrl, this);
    m_ocrBaseUrl->setPlaceholderText(QStringLiteral("例如: https://api.openai.com/v1"));
    m_ocrPrompt = new QTextEdit(this);
    m_ocrPrompt->setText(config.imageToMarkdown.prompt);
    m_ocrPrompt->setPlaceholderText(QStringLiteral("输入图像转换的提示词"));
    m_ocrPrompt->setMaximumHeight(100);

    imageForm->addRow(QStringLiteral("API密钥:"), m_ocrApiKey);
    imageForm->addRow(QStringLiteral("模型:"), m_ocrModel);
    imageForm->addRow(QStringLiteral("基础URL:"), m_ocrBaseUrl);
    imageForm->addRow(QStringLiteral("提示词:"), m_ocrPrompt);
    tabs->addTab(imageTab, QStringLiteral("图像转Markdown"));

    auto* transTab = new QWidget(this);
    auto* transForm = new QFormLayout(transTab);
    m_transApiKey = new QLineEdit(config.translation.apiKey, this);
    m_transApiKey->setPlaceholderText(QStringLiteral("输入OpenAI API密钥"));
    m_transModel = new QLineEdit(config.translation.model, this);
    m_transModel->setPlaceholderText(QStringLiteral("例如: gpt-4"));
    m_transBaseUrl = new QLineEdit(config.translation.baseUrl, this);
    m_transBaseUrl->setPlaceholderText(QStringLiteral("例如: https://api.openai.com/v1"));
    m_transAutoPrompt = new QTextEdit(this);
    m_transAutoPrompt->setText(config.translation.promptAuto);
    m_transAutoPrompt->setPlaceholderText(QStringLiteral("输入自动检测语言时的提示词"));
    m_transAutoPrompt->setMaximumHeight(80);
    m_transScreenshotTargetLang = new QLineEdit(config.translation.screenshotTargetLanguage, this);
    m_transScreenshotTargetLang->setPlaceholderText(QStringLiteral("例如: 简体中文 / English / 日本語"));

    transForm->addRow(QStringLiteral("API密钥:"), m_transApiKey);
    transForm->addRow(QStringLiteral("模型:"), m_transModel);
    transForm->addRow(QStringLiteral("基础URL:"), m_transBaseUrl);
    transForm->addRow(QStringLiteral("自动检测提示词:"), m_transAutoPrompt);
    transForm->addRow(QStringLiteral("截图翻译目标语种:"), m_transScreenshotTargetLang);
    tabs->addTab(transTab, QStringLiteral("文本翻译"));

    auto* uiTab = new QWidget(this);
    auto* uiForm = new QFormLayout(uiTab);
    m_themeMode = new QComboBox(this);
    m_themeMode->addItem(QStringLiteral("暗黑"), QStringLiteral("dark"));
    m_themeMode->addItem(QStringLiteral("明亮"), QStringLiteral("light"));
    m_themeMode->addItem(QStringLiteral("跟随系统"), QStringLiteral("system"));
    {
        const auto t = config.ui.theme.trimmed().toLower();
        if (t == QStringLiteral("light")) {
            m_themeMode->setCurrentIndex(1);
        } else if (t == QStringLiteral("system")) {
            m_themeMode->setCurrentIndex(2);
        } else {
            m_themeMode->setCurrentIndex(0);
        }
    }
    uiForm->addRow(QStringLiteral("主题:"), m_themeMode);
    tabs->addTab(uiTab, QStringLiteral("外观"));

    auto* bgTab = new QWidget(this);
    auto* bgForm = new QFormLayout(bgTab);
    m_startInTray = new QCheckBox(QStringLiteral("启动后最小化到托盘运行"), this);
    m_startInTray->setChecked(config.background.startInTray);
    m_trayBubbles = new QCheckBox(QStringLiteral("启用托盘气泡提示"), this);
    m_trayBubbles->setChecked(config.background.trayBubblesEnabled);

    m_hotkeyMode = new QComboBox(this);
    m_hotkeyMode->addItem(QStringLiteral("共用一个快捷键"), QStringLiteral("shared"));
    m_hotkeyMode->addItem(QStringLiteral("分别设置两个快捷键"), QStringLiteral("separate"));
    const auto mode = config.background.hotkeyMode.trimmed().toLower();
    m_hotkeyMode->setCurrentIndex(mode == QStringLiteral("separate") ? 1 : 0);

    m_hotkeyHint = new QLabel(this);
    m_hotkeyHint->setWordWrap(true);
    m_hotkeyHint->setProperty("class", "hint");

    m_sharedHotkeyAction = new QComboBox(this);
    m_sharedHotkeyAction->addItem(QStringLiteral("截图转Markdown"), QStringLiteral("ocr"));
    m_sharedHotkeyAction->addItem(QStringLiteral("截图翻译"), QStringLiteral("translate"));
    const auto action = config.background.sharedHotkeyAction.trimmed().toLower();
    m_sharedHotkeyAction->setCurrentIndex(action == QStringLiteral("translate") ? 1 : 0);

    m_hotkey = new QLineEdit(config.background.screenshotHotkey, this);
    m_translateHotkey = new QLineEdit(config.background.translateHotkey, this);

    m_enableScreenshotHotkey = new QCheckBox(QStringLiteral("启用截图转Markdown快捷键"), this);
    m_enableScreenshotHotkey->setChecked(config.background.screenshotHotkeyEnabled);
    m_enableTranslateHotkey = new QCheckBox(QStringLiteral("启用截图翻译快捷键"), this);
    m_enableTranslateHotkey->setChecked(config.background.translateHotkeyEnabled);

    bgForm->addRow(QStringLiteral("后台模式:"), m_startInTray);
    bgForm->addRow(QStringLiteral("托盘提示:"), m_trayBubbles);
    bgForm->addRow(QStringLiteral("快捷键模式:"), m_hotkeyMode);
    bgForm->addRow(m_hotkeyHint);
    bgForm->addRow(QStringLiteral("共用快捷键功能:"), m_sharedHotkeyAction);
    bgForm->addRow(QStringLiteral("截图转Markdown快捷键:"), m_hotkey);
    bgForm->addRow(m_enableScreenshotHotkey);
    bgForm->addRow(QStringLiteral("截图翻译快捷键:"), m_translateHotkey);
    bgForm->addRow(m_enableTranslateHotkey);
    tabs->addTab(bgTab, QStringLiteral("后台模式"));

    auto updateEnabled = [this]() {
        const bool separate = m_hotkeyMode->currentData().toString() == QStringLiteral("separate");

        if (separate) {
            m_hotkeyHint->setText(QStringLiteral("分别设置模式：可分别配置两个快捷键，并可单独启用/禁用。"));
        } else {
            m_hotkeyHint->setText(
                QStringLiteral("共用模式：只会注册一个全局快捷键，触发的功能由“共用快捷键功能”决定；下方“截图翻译快捷键/启用开关”在该模式下不生效。"));
        }

        const auto disabledTip = QStringLiteral("当前为“共用一个快捷键”模式：该项不生效。如需分别设置，请切换到“分别设置两个快捷键”。");
        m_translateHotkey->setToolTip(separate ? QString() : disabledTip);
        m_enableScreenshotHotkey->setToolTip(separate ? QString() : disabledTip);
        m_enableTranslateHotkey->setToolTip(separate ? QString() : disabledTip);

        m_sharedHotkeyAction->setEnabled(!separate);
        m_translateHotkey->setEnabled(separate);
        m_enableScreenshotHotkey->setEnabled(separate);
        m_enableTranslateHotkey->setEnabled(separate);
    };
    connect(m_hotkeyMode, &QComboBox::currentIndexChanged, this, [updateEnabled](int) { updateEnabled(); });
    updateEnabled();

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AppConfig ConfigDialog::config() const
{
    AppConfig cfg = m_initial;
    cfg.imageToMarkdown.apiKey = m_ocrApiKey->text();
    cfg.imageToMarkdown.model = m_ocrModel->text();
    cfg.imageToMarkdown.baseUrl = m_ocrBaseUrl->text();
    cfg.imageToMarkdown.prompt = m_ocrPrompt->toPlainText();

    cfg.translation.apiKey = m_transApiKey->text();
    cfg.translation.model = m_transModel->text();
    cfg.translation.baseUrl = m_transBaseUrl->text();
    cfg.translation.promptAuto = m_transAutoPrompt->toPlainText();
    cfg.translation.screenshotTargetLanguage = m_transScreenshotTargetLang->text();

    cfg.ui.theme = m_themeMode->currentData().toString();

    cfg.background.startInTray = m_startInTray->isChecked();
    cfg.background.screenshotHotkey = m_hotkey->text();
    cfg.background.translateHotkey = m_translateHotkey->text();
    cfg.background.hotkeyMode = m_hotkeyMode->currentData().toString();
    cfg.background.sharedHotkeyAction = m_sharedHotkeyAction->currentData().toString();
    cfg.background.screenshotHotkeyEnabled = m_enableScreenshotHotkey->isChecked();
    cfg.background.translateHotkeyEnabled = m_enableTranslateHotkey->isChecked();
    cfg.background.trayBubblesEnabled = m_trayBubbles->isChecked();
    return cfg;
}
