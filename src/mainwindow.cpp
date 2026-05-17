#include "mainwindow.h"

#include "configdialog.h"
#include "configmanager.h"
#include "app_style.h"
#include "globalhotkey.h"
#include "image_to_markdown_runner.h"
#include "image_to_translation_runner.h"
#include "imagetab.h"
#include "openai_chat_client.h"
#include "region_capture_overlay.h"
#include "rounded_menu.h"
#include "translationtab.h"

#include <QApplication>
#include <QAction>
#include <QClipboard>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QFont>
#include <QGuiApplication>
#include <QCursor>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QScreen>
#include <QStyleHints>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTabWidget>
#include <QTimer>
#include <QPainter>
#include <QSvgRenderer>
#include <QHBoxLayout>
#include <QWidget>

static QIcon makeRotatedSvgIcon(const QString& resourcePath, qreal angleDegrees)
{
    const QList<QSize> sizes = {QSize(16, 16), QSize(20, 20), QSize(24, 24), QSize(32, 32)};
    QIcon icon;

    QSvgRenderer renderer(resourcePath);
    if (!renderer.isValid()) {
        return icon;
    }

    for (const auto& size : sizes) {
        QPixmap pix(size);
        pix.fill(Qt::transparent);

        QPainter painter(&pix);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.translate(size.width() / 2.0, size.height() / 2.0);
        painter.rotate(angleDegrees);
        painter.translate(-size.width() / 2.0, -size.height() / 2.0);
        renderer.render(&painter, QRectF(QPointF(0, 0), QSizeF(size)));
        painter.end();

        icon.addPixmap(pix);
    }

    return icon;
}

MainWindow::MainWindow(const AppConfig& config, QWidget* parent)
    : QMainWindow(parent)
    , m_config(config)
{
    setWindowTitle(QStringLiteral("ClipFlow"));
    setGeometry(100, 100, 900, 700);
    setMinimumSize(800, 600);

    m_configPath = QCoreApplication::applicationDirPath() + QStringLiteral("/config.json");
    m_configManager = new ConfigManager(this);

    auto* nam = new QNetworkAccessManager(this);
    m_chatClient = new OpenAIChatClient(nam, this);
    m_imageRunner = new ImageToMarkdownRunner(m_chatClient, this);
    m_translateRunner = new ImageToTranslationRunner(m_chatClient, this);

    auto* tabs = new QTabWidget(this);
    tabs->setFont(QFont(QStringLiteral("Microsoft YaHei UI"), 10));

    m_imageTab = new ImageTab(m_chatClient, this);
    m_translationTab = new TranslationTab(m_chatClient, this);
    m_imageTab->setConfig(m_config);
    m_translationTab->setConfig(m_config);

    tabs->addTab(m_imageTab, QStringLiteral("图像转Markdown"));
    tabs->addTab(m_translationTab, QStringLiteral("文本翻译"));

    auto* settingsButton = new QPushButton(QStringLiteral("配置"), this);
    settingsButton->setToolTip(QStringLiteral("设置"));
    settingsButton->setObjectName(QStringLiteral("btn_settings"));
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openConfigDialog);
    auto* corner = new QWidget(this);
    corner->setObjectName(QStringLiteral("corner_container"));
    auto* cornerLayout = new QHBoxLayout(corner);
    cornerLayout->setContentsMargins(0, 6, 6, 0);
    cornerLayout->addWidget(settingsButton);
    tabs->setCornerWidget(corner, Qt::TopRightCorner);

    setCentralWidget(tabs);
    applyStyles();
    connect(QGuiApplication::styleHints(),
            &QStyleHints::colorSchemeChanged,
            this,
            [this](Qt::ColorScheme) {
                if (m_config.ui.theme.trimmed().toLower() == QStringLiteral("system")) {
                    applyStyles();
                }
            });

    setupTray();
    updateHotkeys();

    connect(m_imageRunner, &ImageToMarkdownRunner::success, this, [this](const QString& markdown) {
        setBusy(false);
        QApplication::clipboard()->setText(markdown);
        if (m_trayIcon && m_config.background.trayBubblesEnabled) {
            m_trayIcon->showMessage(QStringLiteral("ClipFlow"), QStringLiteral("Markdown 已写入剪贴板"));
        }
    });
    connect(m_imageRunner, &ImageToMarkdownRunner::failed, this, [this](const QString& error) {
        setBusy(false);
        if (m_trayIcon) {
            m_trayIcon->showMessage(QStringLiteral("ClipFlow"), QStringLiteral("转换失败: %1").arg(error));
        }
    });
    connect(m_imageRunner, &ImageToMarkdownRunner::canceled, this, [this]() {
        setBusy(false);
        if (m_trayIcon && m_config.background.trayBubblesEnabled) {
            m_trayIcon->showMessage(QStringLiteral("ClipFlow"), QStringLiteral("已取消当前任务"));
        }
    });

    connect(m_translateRunner, &ImageToTranslationRunner::success, this, [this](const QString& translated) {
        setBusy(false);
        QApplication::clipboard()->setText(translated);
        if (m_trayIcon && m_config.background.trayBubblesEnabled) {
            m_trayIcon->showMessage(QStringLiteral("ClipFlow"), QStringLiteral("翻译已写入剪贴板"));
        }
    });
    connect(m_translateRunner, &ImageToTranslationRunner::failed, this, [this](const QString& error) {
        setBusy(false);
        if (m_trayIcon) {
            m_trayIcon->showMessage(QStringLiteral("ClipFlow"), QStringLiteral("翻译失败: %1").arg(error));
        }
    });
    connect(m_translateRunner, &ImageToTranslationRunner::canceled, this, [this]() {
        setBusy(false);
        if (m_trayIcon && m_config.background.trayBubblesEnabled) {
            m_trayIcon->showMessage(QStringLiteral("ClipFlow"), QStringLiteral("已取消本次翻译"));
        }
    });
}

void MainWindow::openConfigDialog()
{
    ConfigDialog dialog(m_config, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    m_config = dialog.config();
    m_imageTab->setConfig(m_config);
    m_translationTab->setConfig(m_config);
    applyStyles();

    QString err;
    m_configManager->save(m_configPath, m_config, &err);

    updateHotkeys();
}

void MainWindow::applyStyles()
{
    const auto scheme = QGuiApplication::styleHints()->colorScheme();
    const auto theme = resolveTheme(m_config.ui.theme, scheme);
    qApp->setStyleSheet(appStylesheet(theme));
}

void MainWindow::setupTray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    m_trayIcon = new QSystemTrayIcon(this);
    m_idleIcon = QIcon(QStringLiteral(":/assets/streamline-color--ai-edit-spark.svg"));
    m_trayIcon->setIcon(m_idleIcon);
    m_trayIcon->setToolTip(QStringLiteral("ClipFlow"));

    auto* menu = new RoundedMenu(this);
    auto* actToggle = menu->addAction(QStringLiteral("显示/隐藏"));
    auto* actCapture = menu->addAction(QStringLiteral("截图转Markdown"));
    auto* actTranslate = menu->addAction(QStringLiteral("截图翻译"));
    m_cancelAction = menu->addAction(QStringLiteral("取消当前任务"));
    m_cancelAction->setEnabled(false);
    auto* actSettings = menu->addAction(QStringLiteral("配置"));
    menu->addSeparator();
    auto* actQuit = menu->addAction(QStringLiteral("退出"));
    m_trayIcon->setContextMenu(menu);

    connect(actToggle, &QAction::triggered, this, [this]() {
        if (isVisible()) {
            hide();
        } else {
            showNormal();
            raise();
            activateWindow();
        }
    });
    connect(actCapture, &QAction::triggered, this, &MainWindow::triggerScreenshotToMarkdown);
    connect(actTranslate, &QAction::triggered, this, &MainWindow::triggerScreenshotTranslate);
    connect(m_cancelAction, &QAction::triggered, this, [this]() {
        if (m_imageRunner && m_imageRunner->isRunning()) {
            m_imageRunner->cancel();
            return;
        }
        if (m_translateRunner && m_translateRunner->isRunning()) {
            m_translateRunner->cancel();
            return;
        }
    });
    connect(actSettings, &QAction::triggered, this, [this]() {
        showNormal();
        raise();
        activateWindow();
        openConfigDialog();
    });
    connect(actQuit, &QAction::triggered, qApp, &QCoreApplication::quit);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason != QSystemTrayIcon::DoubleClick) {
            return;
        }
        if (isVisible()) {
            hide();
        } else {
            showNormal();
            raise();
            activateWindow();
        }
    });

    m_trayIcon->show();
}

void MainWindow::updateHotkeys()
{
    if (!m_sharedHotkey) {
        m_sharedHotkey = new GlobalHotkey(this);
        connect(m_sharedHotkey, &GlobalHotkey::activated, this, &MainWindow::onSharedHotkeyActivated);
    }
    if (!m_screenshotHotkey) {
        m_screenshotHotkey = new GlobalHotkey(this);
        connect(m_screenshotHotkey, &GlobalHotkey::activated, this, &MainWindow::triggerScreenshotToMarkdown);
    }
    if (!m_translateHotkey) {
        m_translateHotkey = new GlobalHotkey(this);
        connect(m_translateHotkey, &GlobalHotkey::activated, this, &MainWindow::triggerScreenshotTranslate);
    }

    m_sharedHotkey->unregisterHotkey();
    m_screenshotHotkey->unregisterHotkey();
    m_translateHotkey->unregisterHotkey();

    const auto mode = m_config.background.hotkeyMode.trimmed().toLower();
    if (mode == QStringLiteral("separate")) {
        if (m_config.background.screenshotHotkeyEnabled) {
            const auto seq = m_config.background.screenshotHotkey;
            if (!m_screenshotHotkey->registerHotkey(seq)) {
                m_screenshotHotkey->registerHotkey(QStringLiteral("Ctrl+Alt+S"));
            }
        }
        if (m_config.background.translateHotkeyEnabled) {
            const auto seq = m_config.background.translateHotkey;
            if (!m_translateHotkey->registerHotkey(seq)) {
                m_translateHotkey->registerHotkey(QStringLiteral("Ctrl+Alt+T"));
            }
        }
        return;
    }

    const auto seq = m_config.background.screenshotHotkey;
    if (!m_sharedHotkey->registerHotkey(seq)) {
        m_sharedHotkey->registerHotkey(QStringLiteral("Ctrl+Alt+S"));
    }
}

void MainWindow::onSharedHotkeyActivated()
{
    const auto action = m_config.background.sharedHotkeyAction.trimmed().toLower();
    if (action == QStringLiteral("translate")) {
        triggerScreenshotTranslate();
        return;
    }
    triggerScreenshotToMarkdown();
}

void MainWindow::triggerScreenshotToMarkdown()
{
    if (!m_imageRunner) {
        return;
    }

    if (m_captureOverlay) {
        return;
    }
    if (m_busy) {
        return;
    }

    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return;
    }

    m_captureOverlay = new RegionCaptureOverlay(screen, nullptr);
    connect(m_captureOverlay, &RegionCaptureOverlay::captured, this, [this](const QImage& image) {
        m_captureOverlay = nullptr;
        setBusy(true);
        if (m_trayIcon && m_config.background.trayBubblesEnabled) {
            m_trayIcon->showMessage(QStringLiteral("ClipFlow"), QStringLiteral("已截取选区，开始识别..."));
        }
        m_imageRunner->run(image, m_config);
    });
    connect(m_captureOverlay, &RegionCaptureOverlay::canceled, this, [this]() { m_captureOverlay = nullptr; });
    connect(m_captureOverlay, &QObject::destroyed, this, [this]() { m_captureOverlay = nullptr; });

    m_captureOverlay->show();
    m_captureOverlay->raise();
    m_captureOverlay->activateWindow();
    m_captureOverlay->setFocus();
}

void MainWindow::triggerScreenshotTranslate()
{
    if (!m_translateRunner) {
        return;
    }

    if (m_captureOverlay) {
        return;
    }
    if (m_busy) {
        return;
    }

    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return;
    }

    m_captureOverlay = new RegionCaptureOverlay(screen, nullptr);
    connect(m_captureOverlay, &RegionCaptureOverlay::captured, this, [this](const QImage& image) {
        m_captureOverlay = nullptr;
        setBusy(true);
        if (m_trayIcon && m_config.background.trayBubblesEnabled) {
            m_trayIcon->showMessage(QStringLiteral("ClipFlow"), QStringLiteral("已截取选区，开始翻译..."));
        }
        m_translateRunner->run(image, m_config);
    });
    connect(m_captureOverlay, &RegionCaptureOverlay::canceled, this, [this]() { m_captureOverlay = nullptr; });
    connect(m_captureOverlay, &QObject::destroyed, this, [this]() { m_captureOverlay = nullptr; });

    m_captureOverlay->show();
    m_captureOverlay->raise();
    m_captureOverlay->activateWindow();
    m_captureOverlay->setFocus();
}

void MainWindow::setBusy(bool busy)
{
    m_busy = busy;
    if (!m_trayIcon) {
        return;
    }

    if (!m_busyTimer) {
        m_busyTimer = new QTimer(this);
        m_busyTimer->setInterval(80);
        connect(m_busyTimer, &QTimer::timeout, this, [this]() {
            if (!m_trayIcon) {
                return;
            }
            m_busyAngle += 15.0;
            if (m_busyAngle >= 360.0) {
                m_busyAngle -= 360.0;
            }
            m_trayIcon->setIcon(makeRotatedSvgIcon(QStringLiteral(":/assets/ic--round-refresh.svg"), m_busyAngle));
        });
    }

    if (m_cancelAction) {
        m_cancelAction->setEnabled(busy);
    }

    if (busy) {
        m_busyAngle = 0.0;
        m_trayIcon->setIcon(makeRotatedSvgIcon(QStringLiteral(":/assets/ic--round-refresh.svg"), m_busyAngle));
        m_trayIcon->setToolTip(QStringLiteral("ClipFlow（识别中）"));
        m_busyTimer->start();
    } else {
        m_busyTimer->stop();
        m_trayIcon->setIcon(m_idleIcon);
        m_trayIcon->setToolTip(QStringLiteral("ClipFlow"));
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        event->ignore();
        hide();
        if (m_config.background.startInTray && m_config.background.trayBubblesEnabled) {
            m_trayIcon->showMessage(QStringLiteral("ClipFlow"), QStringLiteral("已在托盘后台运行"));
        }
        return;
    }
    QMainWindow::closeEvent(event);
}
