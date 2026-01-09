#include "MainWindow.h"
#include "Widgets/SideBar.h"
#include "Widgets/IconButton.h"

#include <QStackedWidget>
#include "Widgets/Metrics.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setMinimumSize(250, 50);
    resize(1600, 900);

    _content = new QStackedWidget(this);
    _sidebar = new SideBar(_content, SideBar::ExpandMode::Hover, this);

    QList<IconButton*> buttons{ QList<IconButton*>() };

    IconButton* explorer = new IconButton(QIcon(":/icons/processor.svg"), "Explorer", _sidebar);
    IconButton* debug = new IconButton(QIcon(":/icons/debug.svg"), "Debug", _sidebar);
    IconButton* scan = new IconButton(QIcon(":/icons/search.svg"), "Scan", _sidebar);
    IconButton* settings = new IconButton(QIcon(":/icons/settings.svg"), "Settings", _sidebar);

    buttons.append(explorer);
    buttons.append(debug);
    buttons.append(scan);
    buttons.append(settings);

    explorer->setCheckable(true);
    debug->setCheckable(true);
    scan->setCheckable(true);

    _sidebar->addTopButton(explorer);
    _sidebar->addTopButton(debug);
    _sidebar->addTopButton(scan);
    _sidebar->addBottomButton(settings);

    for (auto& b : buttons) {
        b->setIconScalePercent(50);
        b->setIconTextSpacing(20);
        b->setPadding({ (Ui::SideBar::buttonHeight - b->iconPaintRect().width()) / 2, 0, 0, 0 });
        b->setProperty("group", "sidebar");
    }
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    _sidebar->setGeometry(0, 0, _sidebar->width(), this->height());
    _content->setGeometry(QRect{ _content->pos(), this->size() });
}
