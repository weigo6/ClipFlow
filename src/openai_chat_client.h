#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

#include <functional>

class OpenAIChatClient : public QObject {
    Q_OBJECT

public:
    explicit OpenAIChatClient(QNetworkAccessManager* networkManager, QObject* parent = nullptr);

    QNetworkReply* postChatCompletions(
        const QUrl& baseUrl,
        const QString& apiKey,
        const QJsonObject& payload,
        int timeoutMs,
        QObject* context,
        std::function<void(QString)> onSuccess,
        std::function<void(QString)> onError);

private:
    QUrl buildChatCompletionsUrl(const QUrl& baseUrl) const;

    QNetworkAccessManager* m_networkManager = nullptr;
};

