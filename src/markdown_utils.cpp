#include "markdown_utils.h"

QString cleanMarkdown(QString text)
{
    QString t = text.trimmed();
    while (t.startsWith('`') && t.endsWith('`') && t.size() >= 2) {
        t = t.mid(1, t.size() - 2).trimmed();
    }

    const auto lower = t.left(8).toLower();
    if (lower == QStringLiteral("markdown")) {
        t = t.mid(8).trimmed();
    }

    return t;
}

