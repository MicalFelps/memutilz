#include "ApplicationWidget.h"

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
{}

ApplicationWidget::~ApplicationWidget() {
	delete d;
}