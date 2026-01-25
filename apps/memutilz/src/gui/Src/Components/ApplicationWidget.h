#pragma once

#include <QWidget>
#include <SARibbonApplicationWidget.h>

namespace Ui {
    class ApplicationWidget;
}

struct ApplicationWidgetPrivate;

class ApplicationWidget : public SARibbonApplicationWidget {
    Q_OBJECT

public:
    explicit ApplicationWidget(SARibbonMainWindow* parent = nullptr);
    ~ApplicationWidget();
private:
    Ui::ApplicationWidget* ui;
    ApplicationWidgetPrivate* d;
    friend struct ApplicationWidgetPrivate;
};