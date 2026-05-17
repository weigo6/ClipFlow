#include "configmanager.h"
#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFont>
#include <QIcon>
#include <QStringList>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setFont(QFont(QStringLiteral("Microsoft YaHei UI"), 10));
    app.setStyle(QStringLiteral("Fusion"));
    app.setWindowIcon(QIcon(QStringLiteral(":/assets/glyphs-poly--sparkles.svg")));

    const auto configPath = QCoreApplication::applicationDirPath() + QStringLiteral("/config.json");
    ConfigManager mgr;
    const auto config = mgr.load(configPath);

    MainWindow w(config);
    const QStringList args = QCoreApplication::arguments();
    const bool trayArg = args.contains(QStringLiteral("--tray")) || args.contains(QStringLiteral("--background"));
    const bool startInTray = trayArg || config.background.startInTray;
    if (startInTray) {
        w.hide();
    } else {
        w.show();
    }

    return app.exec();
}
