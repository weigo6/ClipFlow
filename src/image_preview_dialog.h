#pragma once

#include <QDialog>
#include <QPixmap>

class QLabel;

class ImagePreviewDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImagePreviewDialog(const QPixmap& pixmap, QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateScaled();

    QPixmap m_original;
    QLabel* m_label = nullptr;
};

