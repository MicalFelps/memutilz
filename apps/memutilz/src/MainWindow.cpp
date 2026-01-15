#include <QBoxLayout.h>
#include "MainWindow.h"

struct MainWindowPrivate {
    MainWindow* _this;
    QWidget* _central{ nullptr };
    QHBoxLayout* _layout{ nullptr };

    MainWindowPrivate(MainWindow* _public);
};

MainWindowPrivate::MainWindowPrivate(MainWindow* _public)
    : _this{ _public }
    , _central{ new QWidget(_this) }
    , _layout{ new QHBoxLayout(_central) }
{
}

// ------------------------------------------------------------------

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , d{ new MainWindowPrivate(this) } {

    setMinimumSize(250, 50);
    resize(1600, 900);
}
MainWindow::~MainWindow() { delete d; }

void MainWindow::closeEvent(QCloseEvent* event) {
    QMainWindow::closeEvent(event);
}