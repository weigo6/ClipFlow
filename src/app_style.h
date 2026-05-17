#pragma once

#include <QString>
#include <Qt>

enum class AppTheme {
    Dark,
    Light
};

AppTheme resolveTheme(const QString& themeMode, Qt::ColorScheme systemScheme);
QString appStylesheet(AppTheme theme);

