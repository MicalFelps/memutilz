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
			auto* b = qobject_cast<SidebarToolButton*>(button);

			if (b && b != _selected) {
				emit selectionChanged(b);
				_selected = b;
			}
		});
}
Sidebar::~Sidebar() {}

SidebarToolButton* Sidebar::button(QString id) const {
	const auto buttons = _group->buttons();

	for (auto* b : buttons)
		if (static_cast<SidebarToolButton*>(b)->id() == id)
			return static_cast<SidebarToolButton*>(b);
	return nullptr;
}
void Sidebar::addButton(SidebarToolButton* btn, std::optional<int> pos) {
	const auto buttons = _group->buttons();

	for (auto* b : buttons)
		if (static_cast<SidebarToolButton*>(b)->id() == btn->id())
			return;
	
	int count = _layout->count() - 1; // strech
	int index = pos.has_value() ? qMin(*pos, count) : count;

	_group->addButton(btn);
	_layout->insertWidget(index, btn);
}
void Sidebar::select(SidebarToolButton* btn) {
	for (const auto b : _group->buttons()) {
		if (static_cast<SidebarToolButton*>(b) == btn) {
			btn->setChecked(true);
		}
	}
}