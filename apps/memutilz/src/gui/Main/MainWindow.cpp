#include <QStatusBar>

#include "MainWindow.h"

#include "../Components/ApplicationWidget.h"
#include "../Components/RibbonBar.h"
#include "../Components/DockManager.h"

#include "../Constants/Ids.h"
#include "../Utils/DummyWidget.h"

using namespace Memutilz;

struct MainWindow::Impl {
    Impl() = delete;
    Impl(MainWindow* _public) : _this{_public} { initUI(); }
    void initUI();

    ApplicationWidget* appWidget;
    RibbonBar* ribbonBar;
    DockManager* dockManager;
    QStatusBar* statusBar;

   private:
    MEMUTILZ_DECLARE_PUBLIC(MainWindow)
    void setupApplicationWidget();
    void setupRibbonBar();
    void setupDockWidgets();
    void setupStatusBar();
};

void MainWindow::Impl::initUI() {
    appWidget = new ApplicationWidget(_this);
    ribbonBar = new RibbonBar(_this);
    dockManager = new DockManager(_this);
    statusBar = new QStatusBar(_this);

    _this->setWindowTitle(QString("%1 %2").arg(qApp->applicationName(),
                                               qApp->applicationVersion()));
    setupRibbonBar();
    setupApplicationWidget();
    setupDockWidgets();
    setupStatusBar();
}

void MainWindow::Impl::setupApplicationWidget() {
    appWidget->hide();
    auto btn = ribbonBar->applicationButton();
    _this->connect(btn, &QAbstractButton::clicked, _this,
                   &MainWindow::showApplicationWidget);
}
void MainWindow::Impl::setupRibbonBar() {
    _this->setRibbonBar(ribbonBar);
    _this->setRibbonTheme(SARibbonTheme::RibbonThemePalette);
    ribbonBar->setWindowIcon(QIcon(":/icons/memutilz.ico"));
    ribbonBar->setTitleIconVisible(false);
}
void MainWindow::Impl::setupDockWidgets() {
    auto dock1 = new KDDockWidgets::QtWidgets::DockWidget(
        QStringLiteral("DummyWidget1"));
    auto widget1 = new DummyWidget();
    dock1->setWidget(widget1);

    auto dock2 = new KDDockWidgets::QtWidgets::DockWidget(
        QStringLiteral("DummyWidget2"));
    auto widget2 = new DummyWidget();
    dock2->setWidget(widget2);

    dockManager->setLimit(Constants::DOCK_DUMMY, 2);
    dockManager->addDockWidget(dock1, Constants::DOCK_DUMMY,
                               KDDockWidgets::Location_OnLeft);
    dockManager->addDockWidget(dock2, Constants::DOCK_DUMMY,
                               KDDockWidgets::Location_OnRight);
}
void MainWindow::Impl::setupStatusBar() {
    _this->setStatusBar(statusBar);
    statusBar->showMessage("Ready");
}

//===================================================
// MainWindow
//===================================================

/**
 * This handles all non-UI stuff like process init and so on
 */
MainWindow::MainWindow(const QString& uniqueName, QWidget* parent,
                       bool frameless, KDDockWidgets::MainWindowOptions options,
                       Qt::WindowFlags flags)
    : MainWindowInternal(uniqueName, parent, frameless, options, flags),
      d{std::make_unique<MainWindow::Impl>(this)} {}
MainWindow::~MainWindow() {};

void MainWindow::showApplicationWidget() { d->appWidget->show(); }
