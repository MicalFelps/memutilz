#include "RibbonBar.h"

#include <SARibbonApplicationButton.h>
#include <SARibbonMenu.h>

RibbonBar::RibbonBar(SARibbonBar* ribbon, QWidget* parent)
	: _ribbon{ ribbon }
	, QWidget(parent)
{
	createRibbonApplicationButton();

	// Main Category
	SARibbonCategory* categoryMain = _ribbon->addCategoryPage({ "Main" });
	createCategoryMain(categoryMain);

	// View Category
	SARibbonCategory* categoryView = new SARibbonCategory();
	categoryView->setCategoryName({ "View" });
	categoryView->setObjectName({ "categoryView" });
	createCategoryView(categoryView);
	_ribbon->addCategoryPage(categoryView);

}

void RibbonBar::createRibbonApplicationButton() {
	QAbstractButton* btn = _ribbon->applicationButton();
	if (!btn) {
		btn = new SARibbonApplicationButton(_ribbon);
		_ribbon->setApplicationButton(btn);
	}
	btn->setText(QString("File"));
	connect(btn, &QAbstractButton::clicked,
		this, [this](bool c) { Q_UNUSED(c); emit applicationButtonClicked(); });
}


void RibbonBar::createCategoryMain(SARibbonCategory* page) {
	/*
	Control Flow
		- pause
		- run (continue to x)
		- step into
		- step over
		- step out
	End
		- close
		- detach
		- restart
	*/
	SARibbonPanel* panelFlowControl = page->addPanel({ "Flow Control" });

	QAction* actionDebugPause = createAction({ "Break" }, ":/icons/debug-pause");
	actionDebugPause->setToolTip("");
	panelFlowControl->addLargeAction(actionDebugPause);

	SARibbonMenu* menuDebugContinue{ new SARibbonMenu(this) };
	{
		QAction* action = nullptr;
		QIcon icon = QIcon(":/icons/debug-continue-bugless");

		action = menuDebugContinue->addAction(icon, { "Go Unhandled Exception" });
		action->setObjectName({ "Go Unhandled Exception" });

		action = menuDebugContinue->addAction(icon, { "Go Handled Exception" });
		action->setObjectName({ "Go Handled Exception" });
	}

	QAction* actionDebugContinue = createAction({ "Go" }, ":/icons/debug-continue");
	actionDebugContinue->setToolTip("");
	actionDebugContinue->setMenu(menuDebugContinue);
	panelFlowControl->addLargeAction(actionDebugContinue, QToolButton::MenuButtonPopup);

	QAction* actionDebugStepOut = createAction({ "Step Out" }, ":/icons/debug-step-out");
	QAction* actionDebugStepInto = createAction({ "Step Into" }, ":/icons/debug-step-into");
	QAction* actionDebugStepOver = createAction({ "Step Over" }, ":/icons/debug-step-over");

	panelFlowControl->addSmallAction(actionDebugStepOut);
	panelFlowControl->addSmallAction(actionDebugStepInto);
	panelFlowControl->addSmallAction(actionDebugStepOver);

	// ------------------------------------------------------------

	SARibbonPanel* panelEnd = page->addPanel({ "End" });

	QAction* actionDebugRestart = createAction({ "Restart" }, ":/icons/debug-restart");
	QAction* actionDebugStop = createAction({ "Stop Debugging" }, ":/icons/debug-stop");
	QAction* actionDebugDetach = createAction({ "Detach" }, ":/icons/debug-detach");

	panelEnd->addSmallAction(actionDebugRestart);
	panelEnd->addSmallAction(actionDebugStop);
	panelEnd->addSmallAction(actionDebugDetach);
}
void RibbonBar::createCategoryView(SARibbonCategory* page) {
	SARibbonPanel* panelWindows = new SARibbonPanel({ "Windows" });
	page->addPanel(panelWindows);
}
/*
void RibbonBar::createCategoryTools(SARibbonCategory* page) {
	return;
}
*/


QAction* RibbonBar::createAction(const QString& text, const QString& iconurl, const QString& objName) {
	QAction* a = new QAction(this);
	a->setText(text);
	a->setIcon(QIcon(iconurl));
	a->setObjectName(objName);
	return a;
}
QAction* RibbonBar::createAction(const QString& text, const QString& iconurl) {
	QAction* a = new QAction(this);
	a->setText(text);
	a->setIcon(QIcon(iconurl));
	a->setObjectName(text);
	return a;
}