#include "MainWindow.h"
#include "SidePanel/PanelLeftSide.h"

#include <QLabel>

// #include <QStackedWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    auto* panel_left = new PanelLeftSide(this);
    // panel_left->setOpenEasingCurve(QEasingCurve::OutCubic);
    // panel_left->setCloseEasingCurve(QEasingCurve::OutCubic);
    panel_left->init();

    QLabel* label = new QLabel("Left");
    label->setAlignment(Qt::AlignCenter);

    panel_left->setWidgetResizable(true);
    panel_left->setWidget(label);

}