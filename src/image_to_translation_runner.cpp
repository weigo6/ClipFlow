#include "image_to_translation_runner.h"

#include "openai_chat_client.h"

#include <QBuffer>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>

static QString cleanText(QString s)
{
    s = s.trimmed();
    if (!s.startsWith(QStringLiteral("```")) || !s.endsWith(QStringLiteral("```"))) {
        return s;
    }
    s = s.mid(3);
    s.chop(3);
    s = s.trimmed();
    const int nl = s.indexOf('\n');
    if (nl > 0) {
        const auto first = s.left(nl).trimmed().toLower();
        if (first == QStringLiteral("text") || first == QStringLiteral("markdown") || first == QStringLiteral("plain")) {
            s = s.mid(nl + 1).trimmed();
        }
    }
    return s;
}

ImageToTranslationRunner::ImageToTranslationRunner(OpenAIChatClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
{
}

bool ImageToTranslationRunner::isRunning() const
{
    return m_running;
}

void ImageToTranslationRunner::cancel()
{
    if (!m_running) {
        return;
    }

    m_canceling = true;
    if (m_reply) {
        m_reply->abort();
    } else {
        m_running = false;
        emit canceled();
    }
}

void ImageToTranslationRunner::run(const QImage& image, const AppConfig& cfg)
{
    if (!m_client) {
        emit failed(QStringLiteral("网络模块未初始化"));
        return;
    }
    if (image.isNull()) {
        emit failed(QStringLiteral("请先输入图片"));
        return;
    }

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    const bool ok = image.save(&buffer, "PNG");
    buffer.close();
    if (!ok) {
        emit failed(QStringLiteral("无法处理图像"));
        return;
    }

    const auto target = cfg.translation.screenshotTargetLanguage.trimmed().isEmpty()
                            ? QStringLiteral("简体中文")
                            : cfg.translation.screenshotTargetLanguage.trimmed();
    const auto prompt =
        QStringLiteral("你是一个专业翻译助手。请先识别图片中的文字，然后将其翻译成%1。只输出翻译结果，不要附加解释。")
            .arg(target);

    const auto base64Image = QString::fromLatin1(bytes.toBase64());
    const auto& c = cfg.imageToMarkdown;

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), c.model);

    QJsonArray content;
    content.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("text")}, {QStringLiteral("text"), prompt}});
    content.append(QJsonObject{
        {QStringLiteral("type"), QStringLiteral("image_url")},
        {QStringLiteral("image_url"),
         QJsonObject{{QStringLiteral("url"), QStringLiteral("data:image/png;base64,%1").arg(base64Image)}}}});

    QJsonObject msg;
    msg.insert(QStringLiteral("role"), QStringLiteral("user"));
    msg.insert(QStringLiteral("content"), content);

    payload.insert(QStringLiteral("messages"), QJsonArray{msg});
    payload.insert(QStringLiteral("max_tokens"), 2000);

    m_running = true;
    emit started();

    m_reply = m_client->postChatCompletions(
        QUrl(c.baseUrl),
        c.apiKey,
        payload,
        60000,
        this,
        [this](QString content) {
            m_running = false;
            m_reply = nullptr;
            if (m_canceling) {
                m_canceling = false;
                emit canceled();
                return;
            }
            emit success(cleanText(std::move(content)));
        },
        [this](QString error) {
            m_running = false;
            m_reply = nullptr;
            if (m_canceling) {
                m_canceling = false;
                emit canceled();
                return;
            }
            emit failed(std::move(error));
        });
}

