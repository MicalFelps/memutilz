#include <Utils/Id.h>
#include "Sidebar.h"

Sidebar::Sidebar(QWidget* parent) 
	: QFrame(parent)
	, _group{new QButtonGroup(this)}
	, _layout{new QVBoxLayout(this)}
{
	_group->setExclusive(true);
	_layout->addStretch();

	connect(_group, &QButtonGroup::buttonClicked,
		this, [this](QAbstractButton* button) {
			auto* btn = qobject_cast<SidebarToolButton*>(button);
			if (btn && btn != _selected) {
				emit selectionChanged(btn->id());
				_selected = btn;
			}
		});
}
Sidebar::~Sidebar() {}

void Sidebar::addButton(SidebarToolButton* btn) {
	_group->addButton(btn);
	_layout->insertWidget(_layout->count() - 1, btn);
}
void Sidebar::select(Utils::Id id) {
	const auto& buttons = _group->buttons();
	for (const auto btn : buttons) {
		if (qobject_cast<SidebarToolButton*>(btn)->id() == id) {
			btn->click();
		}
	}
}