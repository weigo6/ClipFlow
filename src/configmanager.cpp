#include "configmanager.h"

#include <QFile>
#include <QJsonDocument>

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
{
}

AppConfig ConfigManager::load(const QString& filePath, QString* errorMessage) const
{
    QFile file(filePath);
    if (!file.exists()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("配置文件不存在");
        }
        return defaultConfig();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法打开配置文件");
        }
        return defaultConfig();
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("配置文件格式错误");
        }
        return defaultConfig();
    }

    return configFromJsonObject(doc.object());
}

bool ConfigManager::save(const QString& filePath, const AppConfig& config, QString* errorMessage) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法写入配置文件");
        }
        return false;
    }

    const auto obj = configToJsonObject(config);
    const QJsonDocument doc(obj);
    const QByteArray data = doc.toJson(QJsonDocument::Indented);
    const auto written = file.write(data);
    file.close();

    if (written != data.size()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("写入配置文件失败");
        }
        return false;
    }

    return true;
}

