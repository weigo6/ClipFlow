#pragma once

#include "config.h"

#include <QWidget>

class OpenAIChatClient;
class QComboBox;
class QPushButton;
class QTextEdit;

class TranslationTab : public QWidget {
    Q_OBJECT

public:
    explicit TranslationTab(OpenAIChatClient* client, QWidget* parent = nullptr);

    void setConfig(const AppConfig& cfg);

private:
    QString translationPrompt() const;
    void translateText();
    void copyTranslation();

    OpenAIChatClient* m_client = nullptr;
    AppConfig m_config;

    QComboBox* m_langCombo = nullptr;
    QTextEdit* m_input = nullptr;
    QTextEdit* m_output = nullptr;
    QPushButton* m_translateButton = nullptr;
};

