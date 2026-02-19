We don't actually care about SARibbonBar, KDDW's `QtWidgets::MainWindow` class fully supports custom menu widgets via `setMenuWidget()` (or `setMenuBar()`), as it inherits (directly or indirectly) from QMainWindow and does not override, restrict, or interfere with any menu-related methods in its source code. The problematic thing is the `systemBar`

SARibbonMainWindow installs this filter:

```cpp
void MainWindowInternal::setRibbonBar(SARibbonBar* ribbonBar) {
    QWidget* oldMenu = QMainWindow::menuWidget();
    if (oldMenu) {
        // If a menu bar has already been set previously, it's gotta go
        oldMenu->deleteLater();
    }
    QMainWindow::setMenuWidget(ribbonBar);
    ribbonBar->setMainWindowStyles(d->ribbonMainWindowStyle);
    const int th = ribbonBar->titleBarHeight();

    if (d->isRibbonFrameUsed()) {
        if (!d->systemButtonBar) {
            d->systemButtonBar =
                RibbonSubElementFactory->createWindowButtonGroup(this);
            d->systemButtonBar->setObjectName(
                QStringLiteral("objSARibbonSystemButtonBar"));
            d->systemButtonBar->setIconSize(QSize(18, 18));

            installEventFilter(d->systemButtonBar);
        }
        SARibbonSystemButtonBar* systemBar = d->systemButtonBar;
        systemBar->setWindowStates(windowState());
        systemBar->setWindowTitleHeight(th);

        // Ensure the sysbar stays on the topmost layer, to prevent it from
        // being covered by the ribbonbar when the ribbonbar is set a second
        // time.
        systemBar->raise();
        systemBar->show();

        ribbonBar->titleIconWidget()->setIcon(windowIcon());
#if SARIBBON_USE_3RDPARTY_FRAMELESSHELPER
        auto fh = d->framelessHelper;
        fh->setTitleBar(ribbonBar);
        // The following widgets/windows need to be allowed to receive mouse
        // clicks.
        fh->setHitTestVisible(systemBar);                         // IMPORTANT!
        fh->setHitTestVisible(ribbonBar->ribbonTabBar());         // IMPORTANT!
        fh->setHitTestVisible(ribbonBar->rightButtonGroup());     // IMPORTANT!
        fh->setHitTestVisible(ribbonBar->applicationButton());    // IMPORTANT!
        fh->setHitTestVisible(ribbonBar->quickAccessBar());       // IMPORTANT!
        fh->setHitTestVisible(ribbonBar->ribbonStackedWidget());  // IMPORTANT!
        fh->setHitTestVisible(ribbonBar->titleIconWidget());      // IMPORTANT!
#if SARIBBON_ENABLE_SNAP_LAYOUT

        if (systemBar->closeButton()) {
            fh->setSystemButton(QWK::WindowAgentBase::Close,
                                systemBar->closeButton());
        }
        if (systemBar->minimizeButton()) {
            fh->setSystemButton(QWK::WindowAgentBase::Minimize,
                                systemBar->minimizeButton());
        }
        if (systemBar->maximizeButton()) {
            fh->setSystemButton(QWK::WindowAgentBase::Maximize,
                                systemBar->maximizeButton());
        }
#endif
#else
        // Capture events from the ribbon bar
        ribbonBar->installEventFilter(this);

        // Set the window's title bar height
        d->framelessHelper->setTitleHeight(th);
        d->framelessHelper->setRubberBandOnResize(false);
#endif
        // Finally, it must be raised; otherwise, newly added elements will be
        // covered.
        if (d->systemButtonBar) d->systemButtonBar->raise();
    }
    if (!d->eventFilter) {
        d->eventFilter = new MainWindowEventFilter(this);
        installEventFilter(d->eventFilter);
    }
}
```

```cpp
installEventFilter(d->systemButtonBar);
```

Which does this:

```cpp
bool SARibbonSystemButtonBar::eventFilter(QObject* obj, QEvent* event)
{
    if (obj && event) {
        SARibbonMainWindow* mainWindow = qobject_cast< SARibbonMainWindow* >(obj);
        if (!mainWindow) {
            // 所有事件都不消费
            return QFrame::eventFilter(obj, event);
        }
        // SARibbonMainWindow的事件
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
            QRect fr         = mainWindow->geometry();
            QSize wgSizeHint = sizeHint();
            setGeometry(fr.width() - wgSizeHint.width(), 0, wgSizeHint.width(), wgSizeHint.height());
            // 把设置好的尺寸给ribbonbar
            if (ribbonBar) {
                ribbonBar->setSystemButtonGroupSize(size());
            }
        } break;
        case QEvent::WindowStateChange: {
            setWindowStates(mainWindow->windowState());
        } break;
        default:
            break;
        }
    }
    return QFrame::eventFilter(obj, event);
}
```

For one, this breaks because we're doing `SARibbonMainWindow* mainWindow = qobject_cast< SARibbonMainWindow* >(obj);` and that's not the class we're using, so that's one problem, but the other thing is that KDDW is not AT ALL aware of the custom TitleBar.

#### Creating a custom TitleBar (the normal way) with KDDW

KDDockWidgets (KDDW) provides a very clean and structured way to create a custom title bar, but it only works for floating windows and tabbed dock groups. That's your biggest problem.

Either way for floating windows, you'd do something like this:

1. You derive from KDDockWidgets::QtWidgets::TitleBar

```cpp
// MyCustomTitleBar.h
#pragma once
#include <kddockwidgets/qtwidgets/views/TitleBar.h>

class MyCustomTitleBar : public KDDockWidgets::QtWidgets::TitleBar
{
    Q_OBJECT
public:
    explicit MyCustomTitleBar(KDDockWidgets::Core::TitleBar *controller,
                              QWidget *parent = nullptr);

    // Optional overrides you usually want:
    void updateCloseButtonEnabled(bool enabled) override;
    void updateFloatButtonState(bool floating) override;
    void updateTitle() override;  // called when title changes

    // You can add your own widgets, styles, colors, gradients, etc.
    // Example: add a QLabel for icon, QPushButtons for min/max/close, etc.
};
```

In then .cpp you then usually
- Call base class constructor
- Create your UI (QHBoxLayout usually)
- Connect to signals from the controller (e.g. titleChanged(), closeClicked(), floatClicked())
- Style it however you want (QSS, palette, custom painting)

```cpp
MyCustomTitleBar::MyCustomTitleBar(Core::TitleBar *controller, QWidget *parent)
    : KDDockWidgets::QtWidgets::TitleBar(controller, parent)
{
    auto lay = new QHBoxLayout(this);
    lay->setContentsMargins(6, 0, 6, 0);
    lay->setSpacing(4);

    // Example: custom look
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(45, 45, 55));
    setPalette(pal);

    // You get these from base class or create your own
    m_titleLabel = new QLabel(title());
    m_titleLabel->setStyleSheet("color: white; font-weight: bold;");
    lay->addWidget(m_titleLabel);

    lay->addStretch();

    // Close button (base already has one, but you can hide/recreate)
    // ...
}
```

2. Derive from KDDockWidgets::QtWidgets::ViewFactory

```cpp
// MyViewFactory.h
#include <kddockwidgets/qtwidgets/views/ViewFactory.h>

class MyViewFactory : public KDDockWidgets::QtWidgets::ViewFactory
{
public:
    KDDockWidgets::QtWidgets::TitleBar *createTitleBar(
        KDDockWidgets::Core::TitleBar *controller,
        QWidget *parent = nullptr) const override
    {
        return new MyCustomTitleBar(controller, parent);
    }

    // You can also override other widgets if desired:
    // TabBar*, TabWidget*, Frame*, etc.
};
```

3. Tell KDDW to use your factory

Do this early, before creating any MainWindow or DockWidget:

```cpp
#include <kddockwidgets/Config.h>

KDDockWidgets::Config::self().setViewFactory(new MyViewFactory());
```

That's basically it for floating windows and grouped/tabbed docks.

##### Important Flags You Usually Combine With Custom Title Bars

These are set via KDDockWidgets::Config::self().setFlags(), do this early too:

| Flag                                    | Purpose when using custom title bar                                                        | Typical value when customizing |
| --------------------------------------- | ------------------------------------------------------------------------------------------ | ------------------------------ |
| `Flag_NativeTitleBar`                   | Forces native OS title bar on floating windows (usually **disable** if you want custom)    | **Off**                        |
| `Flag_HideTitleBarWhenTabsVisible`      | Hides title bar completely when multiple tabs; drag via tab bar (very common + clean look) | **On** (highly recommended)    |
| `Flag_AlwaysTitleBarWhenFloating`       | Forces title bar even for single-tab floating windows (overrides hide flag)                | Usually **Off**                |
| `Flag_AlwaysShowsTabsOnTopLevelWindows` | Shows tabs even on floating windows (instead of title bar)                                 | Sometimes On                   |
| `Flag_NativeDragging`                   | Uses native drag (better Aero-snap/resizing on Windows)                                    | Usually **On**                 |
```cpp
using namespace KDDockWidgets;

Config::self().setFlags(
    Config::Flag_HideTitleBarWhenTabsVisible |
    Config::Flag_NativeDragging
    // | Config::Flag_AlwaysTitleBarWhenFloating   // if you want title even for single dock
);

Config::self().setViewFactory(new MyViewFactory());
```

##### Other Things You Often Need / Want to Do

1. **Dragging the window** Your custom TitleBar usually handles this automatically because the base TitleBar class does: it calls controller()->onTitleBarMouseEvent() which triggers the global DragController. → You rarely need to reimplement mouse events unless you have a non-rectangular drag area.
2. **No extra event filters needed** KDDW already installs what is required (via DragController, etc.). You **don't** need to install your own global filter in most cases.
3. **Aero-snap / resize on Windows** Works very well even with custom title bars — KDDW uses win32 API internally to make resize borders and snap work (one of its killer features). Just keep Flag_NativeDragging on.
4. **Main window frameless (separate topic)** If you also want the **main** KDDockWidgets::QtWidgets::MainWindow to be frameless, that's **not** handled by the factory (the factory only affects floating/group title bars). You do that manually:

```cpp
mainWindow->setWindowFlags(Qt::FramelessWindowHint);
mainWindow->setAttribute(Qt::WA_TranslucentBackground); // optional
```

Then you implement your own title bar as a normal widget at the top.


### What about MainWindow?

The main window itself usually keeps a native title bar (unless you manually set Qt::FramelessWindowHint on it), but floating windows and grouped/tabbed docks can have fully custom title bars.

Dock Widgets can have custom title bars (because they are QT widgets)
Floating Windows can have custom-drawn title bars
the *main window* CANNOT have a custom title bar, because it inherits from QMainWindow, and uses the OS frame by default.

- Dock widgets have custom title bars (because they are Qt widgets).
- Floating windows can have custom-drawn title bars.
- But the _main window_ (since it inherits `QMainWindow`) uses the **native OS frame by default**.

so

### How People Actually Achieve a Custom/Frameless Main Window with KDDW?

You do it the same way you would with any QMainWindow — manually, outside of KDDW's system:

```cpp
class MyMainWindow : public KDDockWidgets::QtWidgets::MainWindow
{
public:
    MyMainWindow(const QString &uniqueName, QWidget *parent = nullptr)
        : KDDockWidgets::QtWidgets::MainWindow(uniqueName, parent)
    {
        // 1. Remove native title bar
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        // Optional: for rounded corners / transparency effects
        setAttribute(Qt::WA_TranslucentBackground);

        // 2. Create your own custom title bar widget (can include SARibbonBar, buttons, etc.)
        m_customTitleBar = new MyCustomTitleBar(this);  // your QWidget subclass
        // Add it to a layout at the very top
        auto central = new QWidget(this);
        auto lay = new QVBoxLayout(central);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(0);
        lay->addWidget(m_customTitleBar);
        // Then add the KDDW docking area or central content below
        lay->addWidget(createCentralWidgetPlaceholder());  // or your own central setup

        setCentralWidget(central);

        // 3. You now have to implement:
        //    - Window dragging (mousePressEvent / mouseMoveEvent on m_customTitleBar)
        //    - System buttons (minimize, maximize/restore, close)
        //    - Optional: resize borders (hardest part — WM_NCHITTEST on Windows, manual elsewhere)
    }
};
```