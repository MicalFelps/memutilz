#pragma once

#include <QToolBar>
#include <QToolButton>
#include <QMenu>

class ButtonGroupWidget : public QToolBar {
    Q_OBJECT

   public:
    explicit ButtonGroupWidget(QWidget* parent = nullptr);
    virtual ~ButtonGroupWidget() override;

    // Add a QAction with a menu to the Quick Access Toolbar
    void addMenuAction(
        QAction* menuAction,
        QToolButton::ToolButtonPopupMode popupMode = QToolButton::InstantPopup);
    QAction* addMenuAction(
        QMenu* menu,
        QToolButton::ToolButtonPopupMode popupMode = QToolButton::InstantPopup);
};
