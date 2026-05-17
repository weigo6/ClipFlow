#pragma once

#include <QImage>
#include <QLabel>

class DropLabel : public QLabel {
    Q_OBJECT

public:
    explicit DropLabel(QWidget* parent = nullptr);

signals:
    void imageDropped(const QImage& image);
    void clicked();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
};
