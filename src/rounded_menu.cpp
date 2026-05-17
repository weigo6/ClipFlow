#include "rounded_menu.h"

#include <QPainterPath>
#include <QRegion>
#include <QResizeEvent>

static void applyRoundedMask(QWidget* w, int radius)
{
    QPainterPath path;
    path.addRoundedRect(w->rect(), radius, radius);
    w->setMask(QRegion(path.toFillPolygon().toPolygon()));
}

RoundedMenu::RoundedMenu(QWidget* parent)
    : QMenu(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlag(Qt::FramelessWindowHint, true);
    setWindowFlag(Qt::NoDropShadowWindowHint, true);
}

void RoundedMenu::resizeEvent(QResizeEvent* event)
{
    QMenu::resizeEvent(event);
    applyRoundedMask(this, 10);
}

