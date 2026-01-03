#include "MainWindow.h"
#include "Widgets/IconButton.h"

#include <QScrollArea>
// #include <QStackedWidget>

// #include <QStyleOption>

#include <QMenu>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setMinimumSize(250, 50);
    resize(1600, 900);


    _content = new QScrollArea(this);
    dynamic_cast<QScrollArea*>(_content);

    _sidebar = new SideBar(_content, SideBar::ExpandMode::Hover, this);

    QList<IconButton*> buttons{ QList<IconButton*>() };

    IconButton* files = new IconButton(QIcon(":/icons/files.svg"), "Explorer", _sidebar);
    IconButton* view = new IconButton(QIcon(":/icons/eye.svg"), "Memory View", _sidebar);
    IconButton* bottom = new IconButton(QIcon(":/icons/right_arrow.svg"), "right arrow", _sidebar);
    IconButton* settings = new IconButton(QIcon(":/icons/settings.svg"), "Settings", _sidebar);

    buttons.append(files);
    buttons.append(view);
    buttons.append(bottom);
    buttons.append(settings);

    for (auto& b : buttons) {
        b->setIconScalePercent(50);
        b->setIconTextSpacing(20);
        b->setProperty("group", "sidebar");
    }

    files->setCheckable(true);
    view->setCheckable(true);

    // view->setDisabled(true);

    _sidebar->addTopButton(files);
    _sidebar->addTopButton(view);
    _sidebar->addBottomButton(settings);
    _sidebar->addBottomButton(bottom);

    IconButton* test = new IconButton(QIcon(":/icons/3bars.svg"), "Testing", _content);
    test->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    test->resize(50, 75);
    test->move(10, 10);

    QMenu* menu = new QMenu(test);
    QAction* action = menu->addAction("Fake action");
    menu->addAction(action);

    test->setMenu(menu);

    connect(action, &QAction::triggered, this, []() {
        qDebug() << "Fake action triggered!";
        });

    /*
    test->setCheckable(true);
    connect(test, &QToolButton::toggled, this, [](bool checked) {
        // qDebug() << checked;
        });
    */

    // test->setMenu(menu);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    _sidebar->setGeometry(0, 0, _sidebar->width(), this->height());
    _content->setGeometry(QRect{ _content->pos(), this->size() });
}