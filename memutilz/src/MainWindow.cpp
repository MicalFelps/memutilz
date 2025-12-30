#include "MainWindow.h"
#include "Widgets/IconButton.h"
// #include <QStackedWidget>

#include <QStyleOption>

#include <QMenu>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setMinimumSize(250, 50);
    resize(1600, 900);

    IconButton* files = new IconButton(QIcon(":/icons/files.svg"), "bazinga", this);
    files->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    files->setIconScalePercent(80);
    files->setHorizontalPadding(0);
    files->setVerticalPadding(10);
    files->setIconTextSpacing(10);
    files->setPopupMode(QToolButton::MenuButtonPopup); // you'll have to do this
    QMenu* menu = new QMenu(files);
    menu->addAction("Open", this, [] { qDebug() << "Open clicked"; });
    menu->addAction("Rename", this, [] { qDebug() << "Rename clicked"; });
    menu->addAction("Delete", this, [] { qDebug() << "Delete clicked"; });
    files->setMenu(menu);
    //files->setCheckable(true);  // Cleaner than connect
    // Avoid fixed size to let sizeHint() breathe
    files->resize(50, 150);  // Remove or increase width
    files->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

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