#include "openai_chat_client.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QPointer>
#include <QTimer>

OpenAIChatClient::OpenAIChatClient(QNetworkAccessManager* networkManager, QObject* parent)
    : QObject(parent)
    , m_networkManager(networkManager)
{
}

QUrl OpenAIChatClient::buildChatCompletionsUrl(const QUrl& baseUrl) const
{
    QUrl url(baseUrl);
    QString path = url.path();
    if (path.endsWith('/')) {
        path.chop(1);
    }
    path += QStringLiteral("/chat/completions");
    url.setPath(path);
    return url;
}

QNetworkReply* OpenAIChatClient::postChatCompletions(
    const QUrl& baseUrl,
    const QString& apiKey,
    const QJsonObject& payload,
    int timeoutMs,
    QObject* context,
    std::function<void(QString)> onSuccess,
    std::function<void(QString)> onError)
{
    if (!m_networkManager) {
        if (onError) {
            onError(QStringLiteral("网络管理器未初始化"));
        }
        return nullptr;
    }

    const auto url = buildChatCompletionsUrl(baseUrl);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());

    const auto body = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QNetworkReply* reply = m_networkManager->post(req, body);

    QPointer<QNetworkReply> safeReply(reply);
    QTimer* timer = new QTimer(reply);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, reply, [safeReply]() {
        if (safeReply) {
            safeReply->abort();
        }
    });
    timer->start(timeoutMs);

    QObject::connect(reply, &QNetworkReply::finished, context, [safeReply, onSuccess, onError]() mutable {
        if (!safeReply) {
            if (onError) {
                onError(QStringLiteral("请求已失效"));
            }
            return;
        }

        const int httpStatus = safeReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto networkError = safeReply->error();
        const QByteArray bytes = safeReply->readAll();

        safeReply->deleteLater();

        if (networkError != QNetworkReply::NoError) {
            if (onError) {
                const QString message = QStringLiteral("%1 (HTTP %2)")
                                            .arg(safeReply->errorString())
                                            .arg(httpStatus);
                onError(message);
            }
            return;
        }

        QJsonParseError parseError;
        const auto doc = QJsonDocument::fromJson(bytes, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            if (onError) {
                onError(QStringLiteral("解析响应失败"));
            }
            return;
        }

        const auto root = doc.object();
        const auto choices = root.value(QStringLiteral("choices")).toArray();
        if (choices.isEmpty()) {
            if (onError) {
                onError(QStringLiteral("响应缺少 choices"));
            }
            return;
        }

        const auto msg = choices.at(0).toObject().value(QStringLiteral("message")).toObject();
        const auto content = msg.value(QStringLiteral("content")).toString();
        if (content.isEmpty()) {
            if (onError) {
                onError(QStringLiteral("响应缺少 content"));
            }
            return;
        }

        if (onSuccess) {
            onSuccess(content);
        }
    });

    return reply;
}
