#pragma once

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QString>

class GlobalHotkey : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT

public:
    explicit GlobalHotkey(QObject* parent = nullptr);
    ~GlobalHotkey() override;

    bool registerHotkey(const QString& sequence);
    void unregisterHotkey();

signals:
    void activated();

protected:
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

private:
    bool registerWindowsHotkey(const QString& sequence);
    void unregisterWindowsHotkey();

    int m_hotkeyId = 0;
};
