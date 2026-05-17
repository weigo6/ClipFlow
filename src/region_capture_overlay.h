#pragma once

#include <QPoint>
#include <QRect>
#include <QWidget>

class QScreen;

class RegionCaptureOverlay : public QWidget {
    Q_OBJECT

public:
    explicit RegionCaptureOverlay(QScreen* screen, QWidget* parent = nullptr);

signals:
    void captured(const QImage& image);
    void canceled();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QRect normalizedSelection() const;
    void finishCapture();
    void cancelCapture();

    QScreen* m_screen = nullptr;
    QPoint m_start;
    QPoint m_end;
    bool m_selecting = false;
};

