#pragma once

#include "config.h"

#include <QObject>
#include <QString>

class ConfigManager : public QObject {
    Q_OBJECT

public:
    explicit ConfigManager(QObject* parent = nullptr);

    AppConfig load(const QString& filePath, QString* errorMessage = nullptr) const;
    bool save(const QString& filePath, const AppConfig& config, QString* errorMessage = nullptr) const;
};

