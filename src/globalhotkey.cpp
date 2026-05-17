#include "globalhotkey.h"

#include <QApplication>
#include <QKeySequence>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

static QString normalizedHotkey(QString s)
{
    s = s.trimmed();
    if (s.isEmpty()) {
        return QString();
    }
    return QKeySequence(s).toString(QKeySequence::NativeText);
}

#ifdef Q_OS_WIN
static int allocateHotkeyId()
{
    static int next = 0xBEEF;
    if (next == 0) {
        next = 0xBEEF;
    }
    return next++;
}

static bool parseHotkeyWindows(const QString& sequence, UINT* modifiers, UINT* vk)
{
    const auto parts = sequence.split('+', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return false;
    }

    UINT mods = 0;
    QString keyPart;
    for (const auto& raw : parts) {
        const auto p = raw.trimmed();
        const auto lower = p.toLower();
        if (lower == QStringLiteral("ctrl") || lower == QStringLiteral("control")) {
            mods |= MOD_CONTROL;
            continue;
        }
        if (lower == QStringLiteral("alt")) {
            mods |= MOD_ALT;
            continue;
        }
        if (lower == QStringLiteral("shift")) {
            mods |= MOD_SHIFT;
            continue;
        }
        if (lower == QStringLiteral("win") || lower == QStringLiteral("meta")) {
            mods |= MOD_WIN;
            continue;
        }
        keyPart = p;
    }

    if (keyPart.isEmpty()) {
        return false;
    }

    UINT vkey = 0;
    const auto upper = keyPart.toUpper();
    if (upper.size() == 1) {
        const QChar ch = upper.at(0);
        if (ch.isLetterOrNumber()) {
            vkey = static_cast<UINT>(ch.unicode());
        }
    } else if (upper.startsWith(QStringLiteral("F"))) {
        bool ok = false;
        const int n = upper.mid(1).toInt(&ok);
        if (ok && n >= 1 && n <= 24) {
            vkey = VK_F1 + (n - 1);
        }
    } else if (upper == QStringLiteral("PRINTSCREEN") || upper == QStringLiteral("PRTSC")) {
        vkey = VK_SNAPSHOT;
    }

    if (vkey == 0) {
        return false;
    }

    *modifiers = mods;
    *vk = vkey;
    return true;
}
#endif

GlobalHotkey::GlobalHotkey(QObject* parent)
    : QObject(parent)
{
    qApp->installNativeEventFilter(this);
}

GlobalHotkey::~GlobalHotkey()
{
    unregisterHotkey();
    qApp->removeNativeEventFilter(this);
}

bool GlobalHotkey::registerHotkey(const QString& sequence)
{
    unregisterHotkey();

    const auto normalized = normalizedHotkey(sequence);
    if (normalized.isEmpty()) {
        return false;
    }

#ifdef Q_OS_WIN
    return registerWindowsHotkey(normalized);
#else
    return false;
#endif
}

void GlobalHotkey::unregisterHotkey()
{
#ifdef Q_OS_WIN
    unregisterWindowsHotkey();
#endif
}

bool GlobalHotkey::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result)
{
    (void)result;

#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        if (msg && msg->message == WM_HOTKEY && static_cast<int>(msg->wParam) == m_hotkeyId) {
            emit activated();
            return true;
        }
    }
#endif

    return false;
}

#ifdef Q_OS_WIN
bool GlobalHotkey::registerWindowsHotkey(const QString& sequence)
{
    UINT mods = 0;
    UINT vkey = 0;
    if (!parseHotkeyWindows(sequence, &mods, &vkey)) {
        return false;
    }

    m_hotkeyId = allocateHotkeyId();
    const BOOL ok = RegisterHotKey(nullptr, m_hotkeyId, mods, vkey);
    if (!ok) {
        m_hotkeyId = 0;
        return false;
    }

    return true;
}

void GlobalHotkey::unregisterWindowsHotkey()
{
    if (m_hotkeyId == 0) {
        return;
    }
    UnregisterHotKey(nullptr, m_hotkeyId);
    m_hotkeyId = 0;
}
#endif
