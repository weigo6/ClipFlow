#pragma once

#include "config.h"

#include <QImage>
#include <QWidget>

class DropLabel;
class ImageToMarkdownRunner;
class OpenAIChatClient;
class QPushButton;
class QTextEdit;

class ImageTab : public QWidget {
    Q_OBJECT

public:
    explicit ImageTab(OpenAIChatClient* client, QWidget* parent = nullptr);

    void setConfig(const AppConfig& cfg);

private:
    void showImage(const QImage& image);
    void openPreview();
    void uploadImage();
    void pasteImage();
    void clearImage();
    void convertImage();
    void copyMarkdown();

    OpenAIChatClient* m_client = nullptr;
    ImageToMarkdownRunner* m_runner = nullptr;
    AppConfig m_config;

    DropLabel* m_dropLabel = nullptr;
    QTextEdit* m_outputText = nullptr;
    QPushButton* m_convertButton = nullptr;
    QImage m_currentImage;
    bool m_hasImage = false;
};
