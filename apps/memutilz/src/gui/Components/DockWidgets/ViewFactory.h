#pragma once

#include <kddockwidgets/qtwidgets/ViewFactory.h>

class DockComponentFactory : public KDDockWidgets::QtWidgets::ViewFactory {
    Q_OBJECT
   public:
    virtual KDDockWidgets::Core::View* createTitleBar(
        KDDockWidgets::Core::TitleBar* controller,
        KDDockWidgets::Core::View* parent) const override;
    QIcon iconForButtonType(KDDockWidgets::TitleBarButtonType type,
                            qreal dpr) const override;
    KDDockWidgets::Core::View* createGroup(KDDockWidgets::Core::Group*,
                                            KDDockWidgets::Core::View* parent) const override;
    // KDDockWidgets::Core::View* createRubberBand(
    //     KDDockWidgets::Core::View* parent) const override;
};
