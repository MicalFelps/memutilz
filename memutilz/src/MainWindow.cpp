#include "MainWindow.h"
#include "Widgets/IconButton.h"

#include <QStackedWidget>
#include <QMenu>
#include "Widgets/Metrics.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setMinimumSize(250, 50);
    resize(1600, 900);

    _content = new QStackedWidget(this);
    _sidebar = new SideBar(_content, SideBar::ExpandMode::Hover, this);

    QList<IconButton*> buttons{ QList<IconButton*>() };

    IconButton* files = new IconButton(QIcon(":/icons/file.svg"), "Files", _sidebar);
    IconButton* view = new IconButton(QIcon(":/icons/eye.svg"), "Memory View", _sidebar);
    IconButton* settings = new IconButton(QIcon(":/icons/settings.svg"), "Settings", _sidebar);

    buttons.append(files);
    buttons.append(view);
    buttons.append(settings);

    files->setCheckable(true);
    view->setCheckable(true);

    _sidebar->addTopButton(files);
    _sidebar->addTopButton(view);
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