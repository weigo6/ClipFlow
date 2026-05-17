#pragma once

#include "config.h"

#include <QMainWindow>

class ConfigManager;
class GlobalHotkey;
class ImageTab;
class ImageToMarkdownRunner;
class ImageToTranslationRunner;
class OpenAIChatClient;
class RegionCaptureOverlay;
class TranslationTab;
class QSystemTrayIcon;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const AppConfig& config, QWidget* parent = nullptr);

private:
    void openConfigDialog();
    void applyStyles();
    void setupTray();
    void updateHotkeys();
    void onSharedHotkeyActivated();
    void triggerScreenshotToMarkdown();
    void triggerScreenshotTranslate();
    void setBusy(bool busy);

protected:
    void closeEvent(QCloseEvent* event) override;

    AppConfig m_config;
    QString m_configPath;

    ConfigManager* m_configManager = nullptr;
    OpenAIChatClient* m_chatClient = nullptr;
    ImageTab* m_imageTab = nullptr;
    TranslationTab* m_translationTab = nullptr;

    QSystemTrayIcon* m_trayIcon = nullptr;
    GlobalHotkey* m_sharedHotkey = nullptr;
    GlobalHotkey* m_screenshotHotkey = nullptr;
    GlobalHotkey* m_translateHotkey = nullptr;
    ImageToMarkdownRunner* m_imageRunner = nullptr;
    ImageToTranslationRunner* m_translateRunner = nullptr;
    RegionCaptureOverlay* m_captureOverlay = nullptr;

    QAction* m_cancelAction = nullptr;
    QTimer* m_busyTimer = nullptr;
    QIcon m_idleIcon;
    qreal m_busyAngle = 0.0;
    bool m_busy = false;
};
