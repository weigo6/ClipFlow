#include "imagetab.h"

#include "droplabel.h"
#include "image_preview_dialog.h"
#include "image_to_markdown_runner.h"
#include "openai_chat_client.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QPixmap>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

ImageTab::ImageTab(OpenAIChatClient* client, QWidget* parent)
    : QWidget(parent)
    , m_client(client)
    , m_config(defaultConfig())
{
    m_runner = new ImageToMarkdownRunner(m_client, this);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(8);

    auto* title = new QLabel(QStringLiteral("图像转Markdown工具"), this);
    title->setProperty("class", "title");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* inputGroup = new QGroupBox(QStringLiteral("图片输入"), this);
    auto* inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(8);

    m_dropLabel = new DropLabel(this);
    m_dropLabel->setText(QStringLiteral("拖拽图片到此处或点击上传按钮"));
    m_dropLabel->setAlignment(Qt::AlignCenter);
    m_dropLabel->setProperty("class", "dropArea");
    m_dropLabel->setMinimumHeight(200);
    m_dropLabel->setCursor(Qt::PointingHandCursor);
    inputLayout->addWidget(m_dropLabel);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    auto* uploadBtn = new QPushButton(QStringLiteral("上传图片"), this);
    auto* pasteBtn = new QPushButton(QStringLiteral("粘贴图片"), this);
    auto* clearBtn = new QPushButton(QStringLiteral("清除"), this);
    btnLayout->addWidget(uploadBtn, 1);
    btnLayout->addWidget(pasteBtn, 1);
    btnLayout->addWidget(clearBtn, 1);
    inputLayout->addLayout(btnLayout);

    layout->addWidget(inputGroup);

    auto* outputGroup = new QGroupBox(QStringLiteral("Markdown输出"), this);
    auto* outputLayout = new QVBoxLayout(outputGroup);
    outputLayout->setContentsMargins(0, 0, 0, 0);
    outputLayout->setSpacing(8);

    m_outputText = new QTextEdit(this);
    m_outputText->setPlaceholderText(QStringLiteral("转换结果将显示在这里..."));
    m_outputText->setReadOnly(false);
    outputLayout->addWidget(m_outputText);

    auto* outputBtnLayout = new QHBoxLayout();
    outputBtnLayout->setSpacing(10);
    m_convertButton = new QPushButton(QStringLiteral("开始转换"), this);
    m_convertButton->setObjectName(QStringLiteral("btn_convert"));
    auto* copyBtn = new QPushButton(QStringLiteral("复制到剪贴板"), this);
    outputBtnLayout->addWidget(m_convertButton, 1);
    outputBtnLayout->addWidget(copyBtn, 1);
    outputLayout->addLayout(outputBtnLayout);

    layout->addWidget(outputGroup);

    connect(m_dropLabel, &DropLabel::imageDropped, this, &ImageTab::showImage);
    connect(m_dropLabel, &DropLabel::clicked, this, &ImageTab::openPreview);
    connect(uploadBtn, &QPushButton::clicked, this, &ImageTab::uploadImage);
    connect(pasteBtn, &QPushButton::clicked, this, &ImageTab::pasteImage);
    connect(clearBtn, &QPushButton::clicked, this, &ImageTab::clearImage);
    connect(m_convertButton, &QPushButton::clicked, this, &ImageTab::convertImage);
    connect(copyBtn, &QPushButton::clicked, this, &ImageTab::copyMarkdown);

    connect(m_runner, &ImageToMarkdownRunner::success, this, [this](const QString& markdown) {
        m_outputText->setPlainText(markdown);
        m_convertButton->setEnabled(true);
    });
    connect(m_runner, &ImageToMarkdownRunner::failed, this, [this](const QString& error) {
        m_outputText->setText(QStringLiteral("转换失败: %1").arg(error));
        m_convertButton->setEnabled(true);
    });
}

void ImageTab::setConfig(const AppConfig& cfg)
{
    m_config = cfg;
}

void ImageTab::showImage(const QImage& image)
{
    m_currentImage = image;
    m_hasImage = !image.isNull();
    const auto scaled = QPixmap::fromImage(image.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_dropLabel->setPixmap(scaled);
    m_dropLabel->setText(QString());
}

void ImageTab::openPreview()
{
    if (!m_hasImage) {
        return;
    }

    const QPixmap pixmap = QPixmap::fromImage(m_currentImage);
    ImagePreviewDialog dialog(pixmap, this);
    dialog.exec();
}

void ImageTab::uploadImage()
{
    const auto path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("选择图片"),
        QString(),
        QStringLiteral("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.webp)"));
    if (path.isEmpty()) {
        return;
    }

    QImage image(path);
    if (image.isNull()) {
        m_dropLabel->setText(QStringLiteral("无法加载图像"));
        return;
    }
    showImage(image);
}

void ImageTab::pasteImage()
{
    auto* clipboard = QApplication::clipboard();
    const auto* mime = clipboard->mimeData();

    if (mime && mime->hasImage()) {
        const auto image = clipboard->image();
        if (!image.isNull()) {
            showImage(image);
            return;
        }
    }

    if (mime && mime->hasUrls()) {
        const auto urls = mime->urls();
        for (const auto& url : urls) {
            if (!url.isLocalFile()) {
                continue;
            }
            const auto path = url.toLocalFile();
            const auto lower = path.toLower();
            if (lower.endsWith(QStringLiteral(".png")) || lower.endsWith(QStringLiteral(".jpg"))
                || lower.endsWith(QStringLiteral(".jpeg")) || lower.endsWith(QStringLiteral(".bmp"))
                || lower.endsWith(QStringLiteral(".gif")) || lower.endsWith(QStringLiteral(".webp"))) {
                QImage image(path);
                if (!image.isNull()) {
                    showImage(image);
                    return;
                }
            }
        }
    }

    if (mime && mime->hasText()) {
        const auto text = clipboard->text().trimmed();
        const auto lower = text.toLower();
        if (lower.endsWith(QStringLiteral(".png")) || lower.endsWith(QStringLiteral(".jpg"))
            || lower.endsWith(QStringLiteral(".jpeg")) || lower.endsWith(QStringLiteral(".bmp"))
            || lower.endsWith(QStringLiteral(".gif")) || lower.endsWith(QStringLiteral(".webp"))) {
            QImage image(text);
            if (!image.isNull()) {
                showImage(image);
                return;
            }
        }
    }

    if (mime) {
        const auto formats = mime->formats();
        for (const auto& fmt : formats) {
            if (!fmt.toLower().contains(QStringLiteral("image"))) {
                continue;
            }
            const auto data = mime->data(fmt);
            QImage image;
            if (image.loadFromData(data) && !image.isNull()) {
                showImage(image);
                return;
            }
        }
    }

    m_dropLabel->setText(QStringLiteral("剪贴板中没有图片或无法识别"));
}

void ImageTab::clearImage()
{
    m_dropLabel->clear();
    m_dropLabel->setText(QStringLiteral("拖拽图片到此处或点击上传按钮"));
    m_outputText->clear();
    m_hasImage = false;
    m_currentImage = QImage();
}

void ImageTab::convertImage()
{
    if (!m_hasImage) {
        m_outputText->setText(QStringLiteral("请先输入图片"));
        return;
    }

    if (!m_runner) {
        m_outputText->setText(QStringLiteral("网络模块未初始化"));
        return;
    }

    m_outputText->setText(QStringLiteral("正在转换中，请稍候..."));
    m_convertButton->setEnabled(false);
    QApplication::processEvents();

    m_runner->run(m_currentImage, m_config);
}

void ImageTab::copyMarkdown()
{
    QApplication::clipboard()->setText(m_outputText->toPlainText());
}
