#include "MainWindow.h"

#include "../Components/RibbonBar.h"
#include "../Components/DockManager.h"

#include "../Constants/Ids.h"
#include "../Utils/DummyWidget.h"

struct MainWindow::Impl {
    Impl() = delete;
    Impl(MainWindow* _public) : _this{_public} {
        dockManager = new DockManager(_this);
    }

    DockManager* dockManager;

   private:
    MEMUTILZ_DECLARE_PUBLIC(MainWindow)
};

//===================================================
// MainWindow
//===================================================

MainWindow::MainWindow(const QString& uniqueName, QWidget* parent,
                       bool frameless, KDDockWidgets::MainWindowOptions options,
                       Qt::WindowFlags flags)
    : MainWindowInternal(uniqueName, parent, frameless, options, flags),
      d{std::make_unique<MainWindow::Impl>(this)} {
    setRibbonBar(new RibbonBar(this));
    setRibbonTheme(SARibbonTheme::RibbonThemePalette);

    auto* mgr = d->dockManager;

    auto dock1 = new KDDockWidgets::QtWidgets::DockWidget(
        QStringLiteral("DummyWidget1"));
    auto widget1 = new DummyWidget();
    dock1->setWidget(widget1);

    auto dock2 = new KDDockWidgets::QtWidgets::DockWidget(
        QStringLiteral("DummyWidget2"));
    auto widget2 = new DummyWidget();
    dock2->setWidget(widget2);

    mgr->setLimit(Core::Constants::Dock::DUMMY, 2);
    mgr->addDockWidget(dock1, Core::Constants::Dock::DUMMY,
                       KDDockWidgets::Location_OnLeft);
    mgr->addDockWidget(dock2, Core::Constants::Dock::DUMMY,
                       KDDockWidgets::Location_OnRight);
}
MainWindow::~MainWindow() {};
