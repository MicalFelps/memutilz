#include "ApplicationWidget.h"

ApplicationWidget::ApplicationWidget(MainWindow* parent)
    : ApplicationWidgetInternal(parent)
    , _layout{ new QHBoxLayout(this) }
    , _sidebar{ new Sidebar(this) }
    , _stack{ new StackedWidget(this) }
{
    initUI();
}
ApplicationWidget::~ApplicationWidget() {}

void ApplicationWidget::initUI() {
    _layout->addWidget(_sidebar);
    _layout->addWidget(_stack);

    _layout->setContentsMargins(10, 10, 10, 10);
    _layout->setSpacing(10);

    SidebarToolButton* buttons[] = {
        new SidebarToolButton("Window-1", QIcon(":/icons/file.svg"), "Window-1", _sidebar),
        new SidebarToolButton("Window-2", QIcon(":/icons/file.svg"), "Window-2", _sidebar),
        new SidebarToolButton("Window-3", QIcon(":/icons/file.svg"), "Window-3", _sidebar),
        new SidebarToolButton("Window-4", QIcon(":/icons/file.svg"), "Window-4", _sidebar),
        new SidebarToolButton("Window-5", QIcon(":/icons/file.svg"), "Window-5", _sidebar),
    };

    for (auto* b : buttons) {
        _mappings[b] = nullptr;
        _sidebar->addButton(b);
        if (b->id() == "Window-2") {
            _sidebar->select(b);
        }
    }
}