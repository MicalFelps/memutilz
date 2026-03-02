#include "ApplicationWidget.h"
#include "ui_ApplicationWidget.h"
#include "../Main/MainWindow.h"

ApplicationWidget::ApplicationWidget(MainWindow* parent)
    : QFrame(parent), ui{new Ui::ApplicationWidget} {
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    ui->setupUi(this);

    parent->installEventFilter(this);
    if (parent) {
        setGeometry(0, 0, parent->width(), parent->height());
    } else {
        setGeometry(0, 0, 300, 300);
    }
}
ApplicationWidget::~ApplicationWidget() { delete ui; }

void ApplicationWidget::resizeToParent(const QSize& parentSize) {
    setGeometry(0, 0, parentSize.width(), parentSize.height());
}

bool ApplicationWidget::eventFilter(QObject* obj, QEvent* ev) {
    if (obj && ev && (obj == parent())) {
        switch (ev->type()) {
            case QEvent::Resize: {
                // Follow the parent window's size changes
                resizeToParent(static_cast<QResizeEvent*>(ev)->size());
                break;
            }
            default:
                break;
        }
    }
    return QFrame::eventFilter(obj, ev);
}
void ApplicationWidget::showEvent(QShowEvent* event) {
    QWidget* par = parentWidget();
    if (par) {
        resizeToParent(par->size());
    }
    raise();
    setFocus();  // Request focus when the window is shown
    QFrame::showEvent(event);
}
void ApplicationWidget::keyPressEvent(QKeyEvent* ev) {
    if (ev) {
        if (ev->key() == Qt::Key_Escape) {
            hide();
            ev->accept();
        }
    }
    return QFrame::keyPressEvent(ev);
}
