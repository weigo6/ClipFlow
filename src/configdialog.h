#pragma once

#include "config.h"

#include <QDialog>

class QLineEdit;
class QTextEdit;
class QCheckBox;
class QComboBox;
class QLabel;

class ConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigDialog(const AppConfig& config, QWidget* parent = nullptr);

    AppConfig config() const;

private:
    AppConfig m_initial;

    QLineEdit* m_ocrApiKey = nullptr;
    QLineEdit* m_ocrModel = nullptr;
    QLineEdit* m_ocrBaseUrl = nullptr;
    QTextEdit* m_ocrPrompt = nullptr;

    QLineEdit* m_transApiKey = nullptr;
    QLineEdit* m_transModel = nullptr;
    QLineEdit* m_transBaseUrl = nullptr;
    QTextEdit* m_transAutoPrompt = nullptr;
    QLineEdit* m_transScreenshotTargetLang = nullptr;

    QComboBox* m_themeMode = nullptr;

    QCheckBox* m_startInTray = nullptr;
    QCheckBox* m_trayBubbles = nullptr;
    QComboBox* m_hotkeyMode = nullptr;
    QLabel* m_hotkeyHint = nullptr;
    QComboBox* m_sharedHotkeyAction = nullptr;
    QLineEdit* m_hotkey = nullptr;
    QCheckBox* m_enableScreenshotHotkey = nullptr;
    QLineEdit* m_translateHotkey = nullptr;
    QCheckBox* m_enableTranslateHotkey = nullptr;
};
