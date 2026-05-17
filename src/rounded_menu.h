#pragma once

#include <QMenu>

class RoundedMenu : public QMenu {
public:
    explicit RoundedMenu(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;
};

