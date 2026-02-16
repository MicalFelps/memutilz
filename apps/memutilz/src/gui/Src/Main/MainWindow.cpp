#include <QStatusBar>

#include "MainWindow.h"

MainWindow::MainWindow(const QString& uniqueName, QWidget* parent,
                       KDDockWidgets::MainWindowOptions options,
                       SARibbonMainWindowStyles style, Qt::WindowFlags flags)
    : MainWindowInternal(uniqueName, parent, options, style, flags) {
    auto* _statusBar = new QStatusBar(this);
    setStatusBar(_statusBar);
    statusBar()->showMessage("Temporary Message");
}
// void MainWindow::setupUi() {
//     setWindowTitle(qApp->applicationName() % " v" %
//     qApp->applicationVersion()); resize(1600, 900); setMinimumSize(250, 100);
//
//     setContentsMargins(1, 0, 1, 0);
//     ribbonBar()->setContentsMargins(4, 0, 4, 0);
//
//     _ribbonBar = new RibbonBar(ribbonBar(), this);
//     _applicationWidget = new ApplicationWidget(this);
//     _applicationWidget->hide();
//
//     _centralDockingArea = new CentralDockingArea(this, d->dockManager);
//
//     connect(_ribbonBar, &RibbonBar::applicationButtonClicked, this, []() {
//         qDebug() << "Clicked!";
//         // _applicationWidget->show();
//     });
//
//     connect(_ribbonBar, &RibbonBar::createTextWidget, this,
//             &MainWindow::handleWindowSignal);
//
//     setRibbonTheme(SARibbonTheme::RibbonThemeDark3);
// }
