#include "ApplicationWidgetInternal.h"
#include "../Constants/Ui.h"
#include "../Main/MainWindow.h"

using namespace Memutilz;

ApplicationWidgetInternal::ApplicationWidgetInternal(MainWindow* parent)
    : QFrame(parent) {
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    parent->installEventFilter(this);
    if (parent) {
        if (parent->isFrameless()) {
            int height = parent->height() - Constants::Ui::FixedMainWindowTitleBarHeight;
            setGeometry(0, Constants::Ui::FixedMainWindowTitleBarHeight, parent->width(), height);
        }
        else {
            setGeometry(0, 0, parent->width(), parent->height());
        }
    }
    else {
        setGeometry(0, 0, 300, 300);
    }
}
ApplicationWidgetInternal::~ApplicationWidgetInternal() {}

void ApplicationWidgetInternal::resizeToParent(const QSize& parentSize) {
    int y = 0;
    int h = parentSize.height();
    if (qobject_cast<MainWindow*>(parentWidget())->isFrameless()) {
        y = Constants::Ui::FixedMainWindowTitleBarHeight;
        h -= Constants::Ui::FixedMainWindowTitleBarHeight;
    }
    setGeometry(0, y, parentSize.width(), h);
}

bool ApplicationWidgetInternal::eventFilter(QObject* obj, QEvent* e) {
    if (obj && e && (obj == parent())) {
        switch (e->type()) {
        case QEvent::Resize: {
            resizeToParent(static_cast<QResizeEvent*>(e)->size());
            break;
        }
        default:
            break;
        }
    }
    return QFrame::eventFilter(obj, e);
}
void ApplicationWidgetInternal::showEvent(QShowEvent* event) {
    QWidget* par = parentWidget();
    if (par) {
        resizeToParent(par->size());
    }
    raise();
    setFocus();  // Request focus when the window is shown
    QFrame::showEvent(event);
}
void ApplicationWidgetInternal::keyPressEvent(QKeyEvent* e) {
    if (e) {
        if (e->key() == Qt::Key_Escape) {
            hide();
            e->accept();
        }
    }
    return QFrame::keyPressEvent(e);
}