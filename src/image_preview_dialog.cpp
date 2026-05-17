#include "image_preview_dialog.h"

#include <QLabel>
#include <QVBoxLayout>

ImagePreviewDialog::ImagePreviewDialog(const QPixmap& pixmap, QWidget* parent)
    : QDialog(parent)
    , m_original(pixmap)
{
    setWindowTitle(QStringLiteral("图片预览"));
    setMinimumSize(800, 600);

    auto* layout = new QVBoxLayout(this);
    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_label);

    updateScaled();
}

void ImagePreviewDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    updateScaled();
}

void ImagePreviewDialog::updateScaled()
{
    if (m_original.isNull() || !m_label) {
        return;
    }

    const QSize target = size() - QSize(40, 40);
    const auto scaled = m_original.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_label->setPixmap(scaled);
}

