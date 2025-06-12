#include "gui/MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new(std::nothrow) Ui::MainWindowClass())
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}