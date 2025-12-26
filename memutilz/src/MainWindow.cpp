#include "MainWindow.h"
#include "Colors.h"
#include "SideBar/SideBar.h"
#include "SideBar/IconButton.h"

#include <QPlainTextEdit>
// #include <QStackedWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setMinimumSize(250, 50);

    _content = new QPlainTextEdit(this);
    dynamic_cast<QPlainTextEdit*>(_content)->setPlainText(QString("AAAAAAAAAAAAAAAAAAAAAAAAAAAA"));

    resize(1600, 900);
    _sidebar = new SideBar(this, SideBar::ExpandMode::Hover, _content);
    _sidebar->setOverlapWithContent(false);

    IconButton* files =     new IconButton(QIcon(":/MainWindow/icons/files.svg"), "Files & Processes", _sidebar);
    IconButton* view =      new IconButton(QIcon(":/MainWindow/icons/eye.svg"), "Memory View", _sidebar);
    IconButton* settings =  new IconButton(QIcon(":/MainWindow/icons/settings.svg"), "Settings", _sidebar);
    settings->setSelectable(false);

    _sidebar->addTopButton(files);
    _sidebar->addTopButton(view);
    _sidebar->addBottomButton(settings);

    _sidebar->initConnections();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    _sidebar->setGeometry(0, 0, _sidebar->width(), this->height());
    _content->setGeometry(QRect{ _content->pos(), this->size() });
}