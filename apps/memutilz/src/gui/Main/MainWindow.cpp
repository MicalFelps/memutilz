#include <QStatusBar>

#include "MainWindow.h"

#include "../Components/RibbonBar.h"
#include "../Components/DockManager.h"

#include "../Constants/Ids.h"
#include "../Utils/DummyWidget.h"

using namespace Memutilz;

struct MainWindow::Impl {
    Impl() = delete;
    Impl(MainWindow* _public) : _this{_public} {
        dockManager = new DockManager(_this);
        statusBar = new QStatusBar(_this);

        _this->setWindowIcon(QIcon(":/icons/memutilz.ico"));
        _this->setRibbonBar(new RibbonBar(_this));
        _this->setRibbonTheme(SARibbonTheme::RibbonThemePalette);
        _this->setStatusBar(statusBar);

        auto dock1 = new KDDockWidgets::QtWidgets::DockWidget(
            QStringLiteral("DummyWidget1"));
        auto widget1 = new DummyWidget();
        dock1->setWidget(widget1);

        auto dock2 = new KDDockWidgets::QtWidgets::DockWidget(
            QStringLiteral("DummyWidget2"));
        auto widget2 = new DummyWidget();
        dock2->setWidget(widget2);

        dockManager->setLimit(Id::Dock::DUMMY, 2);
        dockManager->addDockWidget(dock1, Id::Dock::DUMMY, KDDockWidgets::Location_OnLeft);
        dockManager->addDockWidget(dock2, Id::Dock::DUMMY, KDDockWidgets::Location_OnRight);

        statusBar->showMessage("Ready");
    }

    DockManager* dockManager;
    QStatusBar* statusBar;

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
}
MainWindow::~MainWindow() {};
