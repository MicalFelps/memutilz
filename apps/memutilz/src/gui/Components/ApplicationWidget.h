#pragma once

#include "ApplicationWidgetInternal.h"
#include "ApplicationWidget/Sidebar.h"
#include "ApplicationWidget/StackWidget.h"

class ApplicationWidget : public ApplicationWidgetInternal {
    Q_OBJECT

   public:
    explicit ApplicationWidget(MainWindow* parent);
    virtual ~ApplicationWidget() override;

   private:
    void initUI();
    void createSidebarButtons();

    QHBoxLayout* _layout;
    Sidebar* _sidebar;
    StackedWidget* _stack;

    QHash<Utils::Id, QWidget*> _mappings;
};