#include "ApplicationWidget.h"
#include "../Constants/Ids.h"

using namespace Memutilz;

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

    createSidebarButtons();
}

void ApplicationWidget::createSidebarButtons() {
    struct ButtonInfo {
        Utils::Id id;
        QString iconPath;
        QString text;
    };

    ButtonInfo items[] = {
        {Id::Buttons::SIDEBAR_1, ":/icons/file.svg", "Window-1"},
        {Id::Buttons::SIDEBAR_2, ":/icons/file.svg", "Window-2"},
        {Id::Buttons::SIDEBAR_3, ":/icons/file.svg", "Window-3"},
        {Id::Buttons::SIDEBAR_4, ":/icons/file.svg", "Window-4"},
        {Id::Buttons::SIDEBAR_5, ":/icons/file.svg", "Window-5"}
    };

    for (const auto& item : items) {
        SidebarToolButton* btn = new SidebarToolButton(item.id, QIcon(item.iconPath), item.text, this);
        if (item.id == Id::Buttons::SIDEBAR_2)
            btn->click();
        _mappings[item.id] = btn;
        _sidebar->addButton(btn);
    }
}
