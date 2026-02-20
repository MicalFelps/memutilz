#include <QApplication>
#include <QStyle>

#include "ButtonGroupWidget.h"

//===================================================
// ButtonGroupWidget
//===================================================

ButtonGroupWidget::ButtonGroupWidget(QWidget* parent) : QToolBar(parent) {
    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground);
    setOrientation(Qt::Horizontal);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMovable(false);    // Disable moving
    setFloatable(false);  // Disable floating
    setContentsMargins(0, 0, 0, 0);
    const int smallIconSize =
        QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    setIconSize(QSize(smallIconSize, smallIconSize));
}

ButtonGroupWidget::~ButtonGroupWidget() {}

/**
 * @brief Convenience wrapper around addAction that accounts for popupMode
 */
void ButtonGroupWidget::addMenuAction(
    QAction* menuAction, QToolButton::ToolButtonPopupMode popupMode) {
    if (!menuAction) {
        return;
    }

    addAction(menuAction);

    if (menuAction->menu()) {
        QToolButton* toolButton =
            qobject_cast<QToolButton*>(widgetForAction(menuAction));
        if (toolButton) {
            toolButton->setPopupMode(popupMode);
        }
    }
}

/**
 * @brief Add the QMenu directly, and return the associated QAction.
 */
QAction* ButtonGroupWidget::addMenuAction(
    QMenu* menu, QToolButton::ToolButtonPopupMode popupMode) {
    if (!menu) {
        return nullptr;
    }
    addMenuAction(menu->menuAction(), popupMode);
    return menu->menuAction();
}
