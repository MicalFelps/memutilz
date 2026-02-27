#pragma once

#include <kddockwidgets/qtwidgets/views/Group.h>

class Group : public KDDockWidgets::QtWidgets::Group
{
    Q_OBJECT
public:
    explicit Group(KDDockWidgets::Core::Group* controller,
        QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
};