#include "MainWindow.h"
#include "Widgets/IconButton.h"
// #include <QStackedWidget>

#include <QStyleOption>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setMinimumSize(250, 50);
    resize(1600, 900);

    IconButton* files = new IconButton(QIcon(":/icons/files.svg"), "Explorer", this);
    files->resize(400, 400);

    /*
    

    _content = new QScrollArea(this);
    dynamic_cast<QScrollArea*>(_content);

    resize(1600, 900);
    _sidebar = new SideBar(this, SideBar::ExpandMode::Hover, _content);
    _sidebar->setOverlapWithContent(false);

    IconButton* files =     new IconButton(QIcon(":/icons/files.svg"), "Explorer", _sidebar);
    IconButton* view =      new IconButton(QIcon(":/icons/eye.svg"), "Memory View", _sidebar);
    IconButton* settings =  new IconButton(QIcon(":/icons/settings.svg"), "Settings", _sidebar);
    settings->setSelectable(false);

    _sidebar->addTopButton(files);
    _sidebar->addTopButton(view);
    _sidebar->addBottomButton(settings);

    _sidebar->initConnections();
    */

}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    /*
    _sidebar->setGeometry(0, 0, _sidebar->width(), this->height());
    _content->setGeometry(QRect{ _content->pos(), this->size() });
    */

}