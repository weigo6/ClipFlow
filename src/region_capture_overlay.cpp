#include "region_capture_overlay.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

RegionCaptureOverlay::RegionCaptureOverlay(QScreen* screen, QWidget* parent)
    : QWidget(parent)
    , m_screen(screen)
{
    setWindowFlag(Qt::Tool, true);
    setWindowFlag(Qt::FramelessWindowHint, true);
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::CrossCursor);

    if (m_screen) {
        setGeometry(m_screen->geometry());
    }
}

QRect RegionCaptureOverlay::normalizedSelection() const
{
    QRect r(m_start, m_end);
    return r.normalized();
}

void RegionCaptureOverlay::paintEvent(QPaintEvent* event)
{
    (void)event;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    p.fillRect(rect(), QColor(0, 0, 0, 120));

    const QRect sel = normalizedSelection();
    if (m_selecting && sel.isValid() && sel.width() > 0 && sel.height() > 0) {
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.fillRect(sel, Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

        QPen pen(QColor(0, 120, 215));
        pen.setWidth(2);
        p.setPen(pen);
        p.drawRect(sel.adjusted(1, 1, -1, -1));
    }
}

void RegionCaptureOverlay::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        cancelCapture();
        return;
    }
    m_selecting = true;
    m_start = event->pos();
    m_end = event->pos();
    update();
}

void RegionCaptureOverlay::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_selecting) {
        return;
    }
    m_end = event->pos();
    update();
}

void RegionCaptureOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_selecting || event->button() != Qt::LeftButton) {
        cancelCapture();
        return;
    }
    m_end = event->pos();
    finishCapture();
}

void RegionCaptureOverlay::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        cancelCapture();
        return;
    }
    QWidget::keyPressEvent(event);
}

void RegionCaptureOverlay::finishCapture()
{
    const QRect sel = normalizedSelection();
    if (!m_screen || sel.width() < 5 || sel.height() < 5) {
        cancelCapture();
        return;
    }

    const QPoint topLeftGlobal = mapToGlobal(sel.topLeft());
    const QPoint topLeftLocal = topLeftGlobal - m_screen->geometry().topLeft();

    const auto pix = m_screen->grabWindow(0, topLeftLocal.x(), topLeftLocal.y(), sel.width(), sel.height());
    if (pix.isNull()) {
        cancelCapture();
        return;
    }

    emit captured(pix.toImage());
    close();
}

void RegionCaptureOverlay::cancelCapture()
{
    emit canceled();
    close();
}

