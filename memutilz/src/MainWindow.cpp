#include "MainWindow.h"

#include <QMenuBar>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    
    dockManager = new ads::CDockManager(this);
    setCentralWidget(dockManager);

    createMenuBar();
    createDockWidgets();

    setWindowTitle(QApplication::instance()->applicationName());
}

void MainWindow::createMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu("&File");
}

void MainWindow::createDockWidgets() {
}
