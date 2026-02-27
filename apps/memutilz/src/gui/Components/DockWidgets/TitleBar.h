#pragma once

#include <kddockwidgets/qtwidgets/views/TitleBar.h>

class TitleBar : public KDDockWidgets::QtWidgets::TitleBar {
    Q_OBJECT
   public:
    explicit TitleBar(KDDockWidgets::Core::TitleBar* controller,
                      KDDockWidgets::Core::View* parent = nullptr);
    virtual ~TitleBar() override;

   protected:
    // virtual void paintEvent(QPaintEvent* e) override;
    virtual void init() override;

   private:
    KDDockWidgets::Core::TitleBar* const _controller;
};
