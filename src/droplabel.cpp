#include "droplabel.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QUrl>

DropLabel::DropLabel(QWidget* parent)
    : QLabel(parent)
{
    setAcceptDrops(true);
}

void DropLabel::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasImage() || event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void DropLabel::dropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();
    if (mime->hasImage()) {
        const auto v = mime->imageData();
        if (v.canConvert<QImage>()) {
            const auto image = v.value<QImage>();
            if (!image.isNull()) {
                emit imageDropped(image);
                return;
            }
        }
    }

    if (mime->hasUrls()) {
        const auto urls = mime->urls();
        if (!urls.isEmpty()) {
            const auto url = urls.first();
            if (url.isLocalFile()) {
                const auto path = url.toLocalFile();
                const auto lower = path.toLower();
                if (lower.endsWith(QStringLiteral(".png")) || lower.endsWith(QStringLiteral(".jpg"))
                    || lower.endsWith(QStringLiteral(".jpeg")) || lower.endsWith(QStringLiteral(".bmp"))
                    || lower.endsWith(QStringLiteral(".gif")) || lower.endsWith(QStringLiteral(".webp"))) {
                    QImage image(path);
                    if (!image.isNull()) {
                        emit imageDropped(image);
                        return;
                    }
                }
            }
        }
    }

    setText(QStringLiteral("无效的图像文件"));
}

void DropLabel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}
