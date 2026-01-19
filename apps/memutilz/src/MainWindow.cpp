#include <QMessageBox.h>

#include <SARibbonBar.h>
#include <SARibbonGlobal.h>

#include "MainWindow.h"
#include "ApplicationWidget.h"
#include "CentralDockingArea.h"

struct MainWindowPrivate {
    MainWindow* _this;
    ApplicationWidget* applicationWidget{ nullptr };
    CentralDockingArea* centralDockingArea{ nullptr };

    void setupUi();


    // SARibbon
    void createRibbonApplicationButton();

    void createCategoryDebug(SARibbonCategory* page);
    void createCategoryView(SARibbonCategory* page);
    void createCategoryTools(SARibbonCategory* page);

    void createQuickAccessBar();
    void createRightButtonGroup();
    void createWindowButtonGroupBar();

    QAction* createAction(const QString& text, const QString& iconurl, const QString& objName);
    QAction* createAction(const QString& text, const QString& iconurl);

    // Docking Area
    void createCentralDockingArea();

    // Style
    void setTheme();

    MainWindowPrivate(MainWindow* _public) : _this{ _public } {}
};

void MainWindowPrivate::setupUi() {
    _this->setWindowTitle(qApp->applicationName() % " v" % qApp->applicationVersion());
    _this->resize(1600, 900);

    setTheme();
    
    _this->setRibbonTheme(SARibbonTheme::RibbonThemeDark3);
}

void MainWindowPrivate::setTheme() {
    SARibbonBar* ribbon = _this->ribbonBar();
    ribbon->setWindowTitleTextColor(qApp->palette().text().color());
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