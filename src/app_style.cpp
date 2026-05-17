#include "app_style.h"

static QString normalizeMode(QString s)
{
    s = s.trimmed().toLower();
    if (s == QStringLiteral("light") || s == QStringLiteral("dark") || s == QStringLiteral("system")) {
        return s;
    }
    return QStringLiteral("dark");
}

AppTheme resolveTheme(const QString& themeMode, Qt::ColorScheme systemScheme)
{
    const auto m = normalizeMode(themeMode);
    if (m == QStringLiteral("light")) {
        return AppTheme::Light;
    }
    if (m == QStringLiteral("dark")) {
        return AppTheme::Dark;
    }
    if (systemScheme == Qt::ColorScheme::Dark) {
        return AppTheme::Dark;
    }
    return AppTheme::Light;
}

static QString darkQss()
{
    return QStringLiteral(
        "QDialog { background-color: #1F1F1F; }"
        "QMenu { background-color: #1F1F1F; color: #EAEAEA; border: 1px solid #303030; border-radius: 10px; padding: 6px; }"
        "QMenu::item { padding: 8px 12px; border-radius: 8px; }"
        "QMenu::item:selected { background-color: #2B579A; }"
        "QMenu::separator { height: 1px; background-color: #303030; margin: 6px 8px; }"
        "QMainWindow { background-color: #1F1F1F; }"
        "QTabWidget { background-color: #1F1F1F; color: #E6E6E6; border: none; }"
        "QTabWidget::pane { border: 1px solid #2A2A2A; top: -1px; }"
        "QTabBar::tab { background-color: transparent; color: #CFCFCF; padding: 10px 16px; border: none; border-bottom: 2px solid transparent; margin-right: 10px; }"
        "QTabBar::tab:selected { color: #FFFFFF; border-bottom: 2px solid #0078D4; }"
        "QTabBar::tab:hover:!selected { background-color: rgba(255, 255, 255, 0.06); border-bottom: 2px solid rgba(0, 120, 212, 0.35); }"
        "QGroupBox { border: 1px solid #2A2A2A; border-radius: 10px; margin-top: 10px; padding: 8px; color: #D7D7D7; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
        "QLabel { color: #D7D7D7; }"
        "QLabel[class=\"title\"] { color: #EAEAEA; font-size: 18pt; font-weight: 600; margin: 8px 0 14px 0; }"
        "QLabel[class=\"hint\"] { color: rgba(215, 215, 215, 0.85); font-size: 9pt; }"
        "QLabel[class=\"dropArea\"] { border: 2px dashed #3A8DFF; border-radius: 12px; padding: 20px; background-color: rgba(58, 141, 255, 0.08); qproperty-alignment: AlignCenter; }"
        "QTextEdit, QLineEdit { background-color: #262626; color: #EAEAEA; border: 1px solid #303030; border-radius: 8px; padding: 8px 10px; selection-background-color: #2B579A; }"
        "QTextEdit:focus, QLineEdit:focus { border-color: #3A8DFF; }"
        "QPushButton { background-color: #0078D4; color: #FFFFFF; border: none; border-radius: 8px; padding: 8px 14px; font-weight: 600; }"
        "QPushButton:hover { background-color: #1A86D9; }"
        "QPushButton:pressed { background-color: #0063B1; }"
        "QPushButton:disabled { background-color: #3A3A3A; color: #8A8A8A; }"
        "QPushButton#btn_settings { background-color: transparent; color: #CFCFCF; border: 1px solid #303030; padding: 6px 10px; }"
        "QPushButton#btn_settings:hover { background-color: #2B2B2B; border-color: #3A3A3A; }"
        "QComboBox { background-color: #262626; color: #EAEAEA; border: 1px solid #303030; border-radius: 8px; padding: 6px 34px 6px 10px; }"
        "QComboBox:hover { border-color: #3A8DFF; }"
        "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 30px; border-left: 1px solid #303030; }"
        "QComboBox::down-arrow { image: url(:/assets/ic--round-expand-more-white.svg); width: 14px; height: 14px; }"
        "QComboBoxPrivateContainer { background-color: transparent; border: none; padding: 0px; }"
        "QComboBox QAbstractItemView { background-color: #1F1F1F; color: #EAEAEA; border: 1px solid #303030; border-radius: 10px; outline: 0; selection-background-color: #2B579A; }"
        "QComboBox QAbstractItemView::item { padding: 8px 10px; border-radius: 8px; }"
        "QCheckBox { color: #D7D7D7; }"
        "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; border: 1px solid #4A4A4A; background-color: #262626; }"
        "QCheckBox::indicator:checked { background-color: #0078D4; border-color: #0078D4; image: url(:/assets/ic--round-check.svg); }"
        "QWidget#corner_container { background: transparent; }");
}

static QString lightQss()
{
    return QStringLiteral(
        "QDialog { background-color: #F5F5F5; }"
        "QMenu { background-color: #FFFFFF; color: #202020; border: 1px solid #D6D6D6; border-radius: 10px; padding: 6px; }"
        "QMenu::item { padding: 8px 12px; border-radius: 8px; }"
        "QMenu::item:selected { background-color: #C7E0F4; }"
        "QMenu::separator { height: 1px; background-color: #E0E0E0; margin: 6px 8px; }"
        "QMainWindow { background-color: #F5F5F5; }"
        "QTabWidget { background-color: #F5F5F5; color: #202020; border: none; }"
        "QTabWidget::pane { border: 1px solid #E0E0E0; top: -1px; }"
        "QTabBar::tab { background-color: transparent; color: #303030; padding: 10px 16px; border: none; border-bottom: 2px solid transparent; margin-right: 10px; }"
        "QTabBar::tab:selected { color: #202020; border-bottom: 2px solid #0078D4; }"
        "QTabBar::tab:hover:!selected { background-color: rgba(0, 0, 0, 0.04); border-bottom: 2px solid rgba(0, 120, 212, 0.35); }"
        "QGroupBox { border: 1px solid #E0E0E0; border-radius: 10px; margin-top: 10px; padding: 8px; color: #202020; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
        "QLabel { color: #202020; }"
        "QLabel[class=\"title\"] { color: #202020; font-size: 18pt; font-weight: 600; margin: 8px 0 14px 0; }"
        "QLabel[class=\"hint\"] { color: rgba(32, 32, 32, 0.72); font-size: 9pt; }"
        "QLabel[class=\"dropArea\"] { border: 2px dashed #0078D4; border-radius: 12px; padding: 20px; background-color: rgba(0, 120, 212, 0.08); qproperty-alignment: AlignCenter; }"
        "QTextEdit, QLineEdit { background-color: #FFFFFF; color: #202020; border: 1px solid #D6D6D6; border-radius: 8px; padding: 8px 10px; selection-background-color: #C7E0F4; }"
        "QTextEdit:focus, QLineEdit:focus { border-color: #0078D4; }"
        "QPushButton { background-color: #0078D4; color: #FFFFFF; border: none; border-radius: 8px; padding: 8px 14px; font-weight: 600; }"
        "QPushButton:hover { background-color: #1A86D9; }"
        "QPushButton:pressed { background-color: #0063B1; }"
        "QPushButton:disabled { background-color: #E6E6E6; color: #9A9A9A; }"
        "QPushButton#btn_settings { background-color: transparent; color: #202020; border: 1px solid #D6D6D6; padding: 6px 10px; }"
        "QPushButton#btn_settings:hover { background-color: #EEEEEE; border-color: #CFCFCF; }"
        "QComboBox { background-color: #FFFFFF; color: #202020; border: 1px solid #D6D6D6; border-radius: 8px; padding: 6px 34px 6px 10px; }"
        "QComboBox:hover { border-color: #0078D4; }"
        "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 30px; border-left: 1px solid #D6D6D6; }"
        "QComboBox::down-arrow { image: url(:/assets/ic--round-expand-more-black.svg); width: 14px; height: 14px; }"
        "QComboBoxPrivateContainer { background-color: transparent; border: none; padding: 0px; }"
        "QComboBox QAbstractItemView { background-color: #FFFFFF; color: #202020; border: 1px solid #D6D6D6; border-radius: 10px; outline: 0; selection-background-color: #C7E0F4; }"
        "QComboBox QAbstractItemView::item { padding: 8px 10px; border-radius: 8px; }"
        "QCheckBox { color: #202020; }"
        "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; border: 1px solid #B8B8B8; background-color: #FFFFFF; }"
        "QCheckBox::indicator:checked { background-color: #0078D4; border-color: #0078D4; image: url(:/assets/ic--round-check.svg); }"
        "QWidget#corner_container { background: transparent; }");
}

QString appStylesheet(AppTheme theme)
{
    return theme == AppTheme::Dark ? darkQss() : lightQss();
}
