#ifndef APPLICATIONWIDGET_H
#define APPLICATIONWIDGET_H

#include <QWidget>
#include <SARibbonApplicationWidget.h>

struct ApplicationWidgetPrivate;

class ApplicationWidget : public SARibbonApplicationWidget {
    Q_OBJECT

public:
    explicit ApplicationWidget(SARibbonMainWindow* parent = nullptr);
    ~ApplicationWidget();
private:

    ApplicationWidgetPrivate* d;
    friend struct ApplicationWidgetPrivate;
};

#endif  // APPLICATIONWIDGET_H
