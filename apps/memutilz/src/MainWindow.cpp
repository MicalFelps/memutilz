#include <QMessageBox.h>

#include "MainWindow.h"

MainWindow::MainWindow(QWidget* parent, SARibbonMainWindowStyles style, const Qt::WindowFlags flags)
    : SARibbonMainWindow(parent, style, flags)
{
    setupUi();
}
void MainWindow::closeEvent(QCloseEvent* event) {
    auto r = QMessageBox::question(this
        , "Exit"
        , "Do you really want to exit?"
        , QMessageBox::Yes | QMessageBox::No);

    if (r == QMessageBox::Yes) {
        event->accept();
    }
    else {
        event->ignore();
    }
}

void MainWindow::setupUi() {
    setWindowTitle(qApp->applicationName() % " v" % qApp->applicationVersion());
    resize(1600, 900);
    setMinimumSize(250, 100);
    
    //setContentsMargins(1, 0, 1, 0);
    //ribbonBar()->setContentsMargins(4, 0, 4, 0);

    _ribbonBar = new RibbonBar(ribbonBar(), this);
    _applicationWidget = new ApplicationWidget(this);
    _applicationWidget->hide();
    _centralDockingArea = new CentralDockingArea(this);
    _statusBar = new QStatusBar(this);

    setStatusBar(_statusBar);
    _statusBar->showMessage("Ready");

    connect(_ribbonBar, &RibbonBar::applicationButtonClicked,
        this, [this]() {
            qDebug() << "Clicked!";
            // _applicationWidget->show();
        });

    setRibbonTheme(SARibbonTheme::RibbonThemeDark3);
}