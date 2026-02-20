#include "ApplicationWidget.h"
#include "ui_ApplicationWidget.h"

struct ApplicationWidgetPrivate {
	ApplicationWidget* _this;

	void setupUi();

	ApplicationWidgetPrivate(ApplicationWidget* _public) : _this{ _public } {}
};

void ApplicationWidgetPrivate::setupUi() {
	return;
}

// -----------------------------------------------------------------

ApplicationWidget::ApplicationWidget(SARibbonMainWindow* parent)
	: SARibbonApplicationWidget(parent)
	, d{ new ApplicationWidgetPrivate(this) }
	, ui(new Ui::ApplicationWidget)
{
	ui->setupUi(this);
}

ApplicationWidget::~ApplicationWidget() {
	delete d;
}