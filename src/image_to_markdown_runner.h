#pragma once

#include "config.h"

#include <QImage>
#include <QObject>
#include <QPointer>

class OpenAIChatClient;
class QNetworkReply;

class ImageToMarkdownRunner : public QObject {
    Q_OBJECT

public:
    explicit ImageToMarkdownRunner(OpenAIChatClient* client, QObject* parent = nullptr);

    void run(const QImage& image, const AppConfig& cfg);
    void cancel();
    bool isRunning() const;

signals:
    void started();
    void success(QString markdown);
    void failed(QString error);
    void canceled();

private:
    OpenAIChatClient* m_client = nullptr;
    QPointer<QNetworkReply> m_reply;
    bool m_running = false;
    bool m_canceling = false;
};
