#include <SARibbonBar.h>

#include "SystemButtonBar.h"
#include "ButtonGroupWidget.h"
#include "../../Constants/Ui.h"
#include "../../Main/MainWindowInternal.h"

using namespace Memutilz;

struct SystemButtonBar::Impl {
   private:
    MEMUTILZ_DECLARE_PUBLIC(SystemButtonBar)

   public:
    Impl(SystemButtonBar* _public);

    void setupMinimizeButton(bool on);
    void setupMaximizeButton(bool on);
    void setupCloseButton(bool on);
    void updateSize();
    void resizeElement(QSize size);
    int closeButtonWidthHint() const;
    int maxButtonWidthHint() const;
    int minButtonWidthHint() const;
    QSize sizeHint() const;

    SystemToolButton* buttonClose{nullptr};
    SystemToolButton* buttonMinimize{nullptr};
    SystemToolButton* buttonMaximize{nullptr};
    int closeStretch{3};
    int maxStretch{3};
    int minStretch{3};
    int windowButtonWidth{35};
    int titleBarHeight{Constants::Ui::FixedMainWindowTitleBarHeight};
    Qt::WindowFlags flags{Qt::WindowMinMaxButtonsHint |
                          Qt::WindowCloseButtonHint};
    ButtonGroupWidget* buttonGroup;
};

SystemButtonBar::Impl::Impl(SystemButtonBar* _public)
    : _this{_public}, buttonGroup{new ButtonGroupWidget(_public)} {}

void SystemButtonBar::Impl::setupMinimizeButton(bool on) {
    SystemButtonBar* parent = _this;

    if (on) {
        if (buttonMinimize) {
            buttonMinimize->deleteLater();
            buttonMinimize = nullptr;
        }
        buttonMinimize = new SystemToolButton(parent);
        buttonMinimize->setObjectName(QStringLiteral("MinimizeWindowButton"));
        buttonMinimize->setFocusPolicy(Qt::NoFocus);
        buttonMinimize->show();
        parent->connect(buttonMinimize, &QAbstractButton::clicked, parent,
                        &SystemButtonBar::minimizeWindow);
    } else {
        if (buttonMinimize) {
            buttonMinimize->deleteLater();
            buttonMinimize = nullptr;
        }
    }
    updateSize();
}
void SystemButtonBar::Impl::setupMaximizeButton(bool on) {
    SystemButtonBar* parent = _this;

    if (on) {
        if (buttonMaximize) {
            buttonMaximize->deleteLater();
            buttonMaximize = nullptr;
        }
        buttonMaximize = new SystemToolButton(parent);
        buttonMaximize->setObjectName(QStringLiteral("MaximizeWindowButton"));
        buttonMaximize->setCheckable(true);
        buttonMaximize->setFocusPolicy(Qt::NoFocus);
        buttonMaximize->show();
        parent->connect(buttonMaximize, &QAbstractButton::clicked, parent,
                        &SystemButtonBar::maximizeWindow);
    } else {
        if (buttonMaximize) {
            buttonMaximize->deleteLater();
            buttonMaximize = nullptr;
        }
    }
    updateSize();
}
void SystemButtonBar::Impl::setupCloseButton(bool on) {
    SystemButtonBar* parent = _this;

    if (on) {
        if (buttonClose) {
            buttonClose->deleteLater();
            buttonClose = nullptr;
        }
        buttonClose = new SystemToolButton(parent);
        buttonClose->setObjectName(QStringLiteral("CloseWindowButton"));
        buttonClose->setFocusPolicy(Qt::NoFocus);
        buttonClose->show();
        parent->connect(buttonClose, &QAbstractButton::clicked, parent,
                        &SystemButtonBar::closeWindow);
    } else {
        if (buttonClose) {
            buttonClose->deleteLater();
            buttonClose = nullptr;
        }
    }
    updateSize();
}

void SystemButtonBar::Impl::updateSize() { resizeElement(_this->size()); }
void SystemButtonBar::Impl::resizeElement(QSize size) {
    int x = size.width();
    if (buttonClose) {
        int w = closeButtonWidthHint();
        x -= w;
        buttonClose->setGeometry(x, 0, w, size.height());
    }
    if (buttonMaximize) {
        int w = maxButtonWidthHint();
        x -= w;
        buttonMaximize->setGeometry(x, 0, w, size.height());
    }
    if (buttonMinimize) {
        int w = minButtonWidthHint();
        x -= w;
        buttonMinimize->setGeometry(x, 0, w, size.height());
    }
    if (buttonGroup) {
        buttonGroup->setGeometry(0, 0, x, size.height());
    }
}

int SystemButtonBar::Impl::closeButtonWidthHint() const {
    qreal t = closeStretch + maxStretch + minStretch;
    return qRound((3 * windowButtonWidth) * closeStretch / t);
}
int SystemButtonBar::Impl::maxButtonWidthHint() const {
    qreal t = closeStretch + maxStretch + minStretch;
    return qRound((3 * windowButtonWidth) * maxStretch / t);
}
int SystemButtonBar::Impl::minButtonWidthHint() const {
    qreal t = closeStretch + maxStretch + minStretch;
    return qRound((3 * windowButtonWidth) * minStretch / t);
}
QSize SystemButtonBar::Impl::sizeHint() const {
    int h = titleBarHeight;
    if (h < 20) {
        h = 20;
    }
    QSize r(0, 0);
    if (buttonGroup) {
        r = buttonGroup->sizeHint();
    }
    r.setHeight(h);
    if (buttonClose) {
        r.setWidth(r.width() + closeButtonWidthHint());
    }
    if (buttonMaximize) {
        r.setWidth(r.width() + maxButtonWidthHint());
    }
    if (buttonMinimize) {
        r.setWidth(r.width() + minButtonWidthHint());
    }
    return r;
}

//===================================================
// SystemToolButton
//===================================================

SystemToolButton::SystemToolButton(QWidget* parent) : QToolButton(parent) {
    setAutoRaise(true);
}

//===================================================
// SystemButtonBar
//===================================================

SystemButtonBar::SystemButtonBar(QWidget* parent)
    : QFrame(parent), d{std::make_unique<SystemButtonBar::Impl>(this)} {
    updateWindowFlags();
}
SystemButtonBar::SystemButtonBar(QWidget* parent, Qt::WindowFlags flags)
    : QFrame(parent), d{std::make_unique<SystemButtonBar::Impl>(this)} {
    d->flags = flags;
    updateWindowFlags();
}
SystemButtonBar::~SystemButtonBar() {}

void SystemButtonBar::setupMinimizeButton(bool on) {
    d->setupMinimizeButton(on);
}
void SystemButtonBar::setupMaximizeButton(bool on) {
    d->setupMaximizeButton(on);
}
void SystemButtonBar::setupCloseButton(bool on) { d->setupCloseButton(on); }

QAbstractButton* SystemButtonBar::minimizeButton() const {
    return d->buttonMinimize;
}
QAbstractButton* SystemButtonBar::maximizeButton() const {
    return d->buttonMaximize;
}
QAbstractButton* SystemButtonBar::closeButton() const { return d->buttonClose; }

void SystemButtonBar::updateWindowFlags() {
    QWidget* parentWidget = this;
    while (parentWidget->parentWidget()) {
        parentWidget = parentWidget->parentWidget();
    }
    Qt::WindowFlags flags = parentWidget->windowFlags();
    setupSystemButtons(flags);
}
Qt::WindowFlags SystemButtonBar::windowFlags() const {
    Qt::WindowFlags f = Qt::Widget;

    if (d->flags & Qt::WindowCloseButtonHint) {
        f |= Qt::WindowCloseButtonHint;
    }
    if (d->flags & Qt::WindowMaximizeButtonHint) {
        f |= Qt::WindowMaximizeButtonHint;
    }
    if (d->flags & Qt::WindowMinimizeButtonHint) {
        f |= Qt::WindowMinimizeButtonHint;
    }

    return (f);
}
void SystemButtonBar::setupSystemButtons(Qt::WindowFlags flags) {
    d->flags = flags;
    setupMinimizeButton(flags & Qt::WindowMinimizeButtonHint);
    setupMaximizeButton(flags & Qt::WindowMaximizeButtonHint);
    setupCloseButton(flags & Qt::WindowCloseButtonHint);
}
void SystemButtonBar::updateSystemButtonStates(Qt::WindowStates states) {
    if (d->buttonMaximize) {
        bool on = states.testFlag(Qt::WindowMaximized);
        d->buttonMaximize->setChecked(on);
    }
}

int SystemButtonBar::windowTitleHeight() const { return d->titleBarHeight; }
void SystemButtonBar::setWindowTitleHeight(int h) { d->titleBarHeight = h; }

int SystemButtonBar::windowButtonWidth() const { return d->windowButtonWidth; }
void SystemButtonBar::setWindowButtonWidth(int w) { d->windowButtonWidth = w; }

void SystemButtonBar::setButtonWidthStretch(int close, int max, int min) {
    d->closeStretch = close;
    d->maxStretch = max;
    d->minStretch = min;
}

QSize SystemButtonBar::sizeHint() const { return d->sizeHint(); }

QSize SystemButtonBar::iconSize() const { return d->buttonGroup->iconSize(); }
void SystemButtonBar::setIconSize(const QSize& size) {
    d->buttonGroup->setIconSize(size);
}

void SystemButtonBar::addAction(QAction* a) { d->buttonGroup->addAction(a); }
void SystemButtonBar::addMenuAction(
    QAction* menuAction, QToolButton::ToolButtonPopupMode popupMode) {
    d->buttonGroup->addMenuAction(menuAction, popupMode);
}
QAction* SystemButtonBar::addMenuAction(
    QMenu* menu, QToolButton::ToolButtonPopupMode popupMode) {
    return d->buttonGroup->addMenuAction(menu, popupMode);
}
QAction* SystemButtonBar::addSeparator() {
    return d->buttonGroup->addSeparator();
}
QAction* SystemButtonBar::addWidget(QWidget* w) {
    return d->buttonGroup->addWidget(w);
}

bool SystemButtonBar::eventFilter(QObject* obj, QEvent* event) {
    if (obj && event) {
        MainWindowInternal* mainWindow = qobject_cast<MainWindowInternal*>(obj);
        if (!mainWindow) {
            return QFrame::eventFilter(obj, event);
        }

        switch (event->type()) {
            case QEvent::Resize: {
                int th = 25;

                SARibbonBar* ribbonBar = mainWindow->ribbonBar();
                if (ribbonBar) {
                    th = ribbonBar->titleBarHeight();
                }
                if (th != height()) {
                    setWindowTitleHeight(th);
                }
                QRect frameRect = mainWindow->geometry();
                // widget group size hint
                QSize wgSizeHint = sizeHint();
                setGeometry(frameRect.width() - wgSizeHint.width(), 0,
                            wgSizeHint.width(), wgSizeHint.height());

                if (ribbonBar) {
                    ribbonBar->setSystemButtonGroupSize(size());
                }
            } break;
            case QEvent::WindowStateChange: {
                setWindowState(mainWindow->windowState());
            } break;
            default:
                break;
        }
    }
    return QFrame::eventFilter(obj, event);
}
void SystemButtonBar::resizeEvent(QResizeEvent* e) {
    Q_UNUSED(e)
    d->resizeElement(size());
}

void SystemButtonBar::closeWindow() {
    if (parentWidget()) {
        parentWidget()->close();
    }
}
void SystemButtonBar::minimizeWindow() {
    if (parentWidget()) {
        parentWidget()->showMinimized();
    }
}
void SystemButtonBar::maximizeWindow() {
    QWidget* parent = parentWidget();

    if (parent) {
        if (parent->isMaximized()) {
            parent->showNormal();
        } else {
            parent->showMaximized();
        }
    }
}
