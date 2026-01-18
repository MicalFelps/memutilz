#include <QMessageBox.h>
#include <QFile>

#include <SARibbonBar.h>

#include "MainWindow.h"
#include "ApplicationWidget.h"
#include "CentralDockingArea.h"

struct MainWindowPrivate {
    MainWindow* _this;
    ApplicationWidget* applicationWidget{ nullptr };
    CentralDockingArea* centralDockingArea{ nullptr };

    void setupUi();

    /*
    void createRibbonApplicationButton();

    void createCategoryDebug(SARibbonCategory* page);
    void createCategoryView(SARibbonCategory* page);
    void createCategoryTools(SARibbonCategory* page);

    void createCentralDockingArea();
    */

    void setDarkStyle();

    MainWindowPrivate(MainWindow* _public) : _this{ _public } {}
};

void MainWindowPrivate::setupUi() {
    _this->setWindowTitle(qApp->applicationName() % " v" % qApp->applicationVersion());
    _this->resize(1600, 900);

    setDarkStyle();

}

void MainWindowPrivate::setDarkStyle() {
    SARibbonBar* ribbon = _this->ribbonBar();
    ribbon->setWindowTitleTextColor(qApp->palette().text().color());

    QFile style(":/styles/saribbon.qss");
    style.open(QFile::ReadOnly);
    QString styleSheet = style.readAll();
    ribbon->setStyleSheet(styleSheet);




}

// ------------------------------------------------------------------

MainWindow::MainWindow(QWidget* parent)
    : SARibbonMainWindow(parent)
    , d{ new MainWindowPrivate(this) }
{
    d->setupUi();
}

MainWindow::~MainWindow() {
    delete d;
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