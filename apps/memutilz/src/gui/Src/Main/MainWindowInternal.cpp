#include <SARibbonBar.h>
#include <SARibbonElementManager.h>
#include <SARibbonTabBar.h>
#include <SARibbonUtil.h>
#include <SARibbonTitleIconWidget.h>

#if SARIBBON_USE_3RDPARTY_FRAMELESSHELPER
#include <QWKWidgets/widgetwindowagent.h>
#include <SARibbonButtonGroupWidget.h>
#include <SARibbonQuickAccessBar.h>
#include <SARibbonStackedWidget.h>
#else
#include <SAFramelessHelper.h>
#endif

#include "MainWindowInternal.h"

class SARibbonTitleIconWidget;

//===================================================
// MainWindowInternal::Impl
//===================================================

/**
 * @brief MainWindowInternal Impl class
 */
struct MainWindowInternal::Impl {
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;
    Impl(MainWindowInternal* _public) : _this{_public} {}
    void installFrameless(MainWindowInternal* _public);
    bool isRibbonBarUsed() const;
    bool isRibbonFrameUsed() const;
    bool isNativeFrameUsed() const;
    void checkMainWindowFlag();

    // Theme specific
    static void updateTabBarMargins(SARibbonTabBar* tab, SARibbonTheme theme);
    static void updateContextColors(SARibbonBar* bar, SARibbonTheme theme);
    static void updateTabBarBaseLineColor(SARibbonBar* bar,
                                          SARibbonTheme theme);

   public:
    SARibbonMainWindowStyles ribbonMainWindowStyle;
    SARibbonSystemButtonBar* systemButtonBar{nullptr};
    SARibbonTheme currentRibbonTheme{SARibbonTheme::RibbonThemePalette};

#if SARIBBON_USE_3RDPARTY_FRAMELESSHELPER
    QWK::WidgetWindowAgent* framelessHelper{nullptr};
#else
    SAFramelessHelper* framelessHelper{nullptr};
#endif

    MainWindowEventFilter* eventFilter;

   private:
    MEMUTILZ_DECLARE_PUBLIC(MainWindowInternal)
};

void MainWindowInternal::Impl::installFrameless(MainWindowInternal* _public) {
#if SARIBBON_USE_3RDPARTY_FRAMELESSHELPER
    framelessHelper = new QWK::WidgetWindowAgent(_public);
    framelessHelper->setup(_public);
#else
    framelessHelper = new SAFramelessHelper(_public);
#endif
}

bool MainWindowInternal::Impl::isRibbonBarUsed() const {
    return ribbonMainWindowStyle.testFlag(
        SARibbonMainWindowStyleFlag::UseRibbonMenuBar);
}
bool MainWindowInternal::Impl::isRibbonFrameUsed() const {
    return ribbonMainWindowStyle.testFlag(
        SARibbonMainWindowStyleFlag::UseRibbonFrame);
}
bool MainWindowInternal::Impl::isNativeFrameUsed() const {
    return ribbonMainWindowStyle.testFlag(
        SARibbonMainWindowStyleFlag::UseNativeFrame);
}

/**
 * @brief Checks the validity of the flag configuration
 */
void MainWindowInternal::Impl::checkMainWindowFlag() {
    if (!ribbonMainWindowStyle.testFlag(
            SARibbonMainWindowStyleFlag::UseRibbonFrame) &&
        !ribbonMainWindowStyle.testFlag(
            SARibbonMainWindowStyleFlag::UseNativeFrame)) {
        ribbonMainWindowStyle.setFlag(
            SARibbonMainWindowStyleFlag::UseRibbonFrame, true);
    }

    if (!ribbonMainWindowStyle.testFlag(
            SARibbonMainWindowStyleFlag::UseRibbonMenuBar) &&
        !ribbonMainWindowStyle.testFlag(
            SARibbonMainWindowStyleFlag::UseNativeMenuBar)) {
        ribbonMainWindowStyle.setFlag(
            SARibbonMainWindowStyleFlag::UseRibbonMenuBar, true);
    }
}

void MainWindowInternal::Impl::updateTabBarMargins(SARibbonTabBar* tab,
                                                   SARibbonTheme theme) {
    static const std::map<SARibbonTheme, QMargins> themeMargins = {
        {SARibbonTheme::RibbonThemeWindows7, {5, 0, 0, 0}},
        {SARibbonTheme::RibbonThemeOffice2013, {5, 0, 0, 0}},
        {SARibbonTheme::RibbonThemeOffice2016Blue, {5, 0, 0, 0}},
        {SARibbonTheme::RibbonThemeDark, {5, 0, 0, 0}},
        {SARibbonTheme::RibbonThemeDark2, {5, 0, 0, 0}},
        {SARibbonTheme::RibbonThemeOffice2021Blue, {5, 0, 5, 0}},
        {SARibbonTheme::RibbonThemePalette, {0, 0, 0, 0}}};
    auto it = themeMargins.find(theme);
    if (it != themeMargins.end()) {
        tab->setTabMargin(it->second);
    }
}

void MainWindowInternal::Impl::updateContextColors(SARibbonBar* bar,
                                                   SARibbonTheme theme) {
    static const SARibbonBar::FpContextCategoryHighlight cs_darkerHighlight =
        [](const QColor& c) -> QColor { return c.darker(); };
    static const SARibbonBar::FpContextCategoryHighlight cs_vibrantHighlight =
        [](const QColor& c) -> QColor { return SA::makeColorVibrant(c); };

    switch (theme) {
        case SARibbonTheme::RibbonThemeWindows7:
        case SARibbonTheme::RibbonThemeOffice2013:
        case SARibbonTheme::RibbonThemeDark:
            bar->setContextCategoryColorList({});
            bar->setContextCategoryColorHighLight(cs_vibrantHighlight);
            break;
        case SARibbonTheme::RibbonThemeOffice2016Blue:
            bar->setContextCategoryColorList({QColor(18, 64, 120)});
            bar->setContextCategoryColorHighLight(cs_darkerHighlight);
            break;
        case SARibbonTheme::RibbonThemeOffice2021Blue:
            bar->setContextCategoryColorList({QColor(209, 207, 209)});
            bar->setContextCategoryColorHighLight(
                [](const QColor& c) -> QColor { return QColor(39, 96, 167); });
            break;
        case SARibbonTheme::RibbonThemePalette:
            bar->setContextCategoryColorList(
                {qApp->palette().color(QPalette::Base)});
            bar->setContextCategoryTitleTextColor(
                {qApp->palette().color(QPalette::Text)});
            bar->setContextCategoryColorHighLight(
                [](const QColor& c) -> QColor {
                    return QColor(109, 104, 204);
                });
            break;
        default:
            break;
    }
}

void MainWindowInternal::Impl::updateTabBarBaseLineColor(SARibbonBar* bar,
                                                         SARibbonTheme theme) {
    if (theme == SARibbonTheme::RibbonThemeOffice2013) {
        bar->setTabBarBaseLineColor(QColor(186, 201, 219));
    } else {
        bar->setTabBarBaseLineColor(QColor());
    }
}

//===================================================
// MainWindowInternal
//===================================================

/**
 * @brief constructor for MainWindowInternal
 *
 * The constructor takes care of initializing the main window based
 * on flags passed in (SARibbonMainWindowStyleFlag). It supports many
 * window styles (such as whether to use a ribbon frame or menu bar) and
 * automatically configures window behaviour according to the selected style.
 *
 * @param parent The parent widget.
 * @param style Window style flags that control the window’s appearance
 *              and behavior. The following flag combinations are supported:
 *        - @c SARibbonMainWindowStyleFlag::UseRibbonFrame:
 *              Use the Ribbon custom frame (frameless window)
 *              (enabled by default).
 *        - @c SARibbonMainWindowStyleFlag::UseNativeFrame:
 *              Use the system’s native window frame.
 *        - @c SARibbonMainWindowStyleFlag::UseRibbonMenuBar:
 *              Use a Ribbon-style menu bar (enabled by default).
 *        - @c SARibbonMainWindowStyleFlag::UseNativeMenuBar:
 *              Use the system’s native (non-ribbon) menu bar.
 *        Flags can be combined using the bitwise OR operator (|), for example:
 *        @c SARibbonMainWindowStyleFlag::UseRibbonFrame |
 *           SARibbonMainWindowStyleFlag::UseRibbonMenuBar
 *
 * @param flags Standard Qt window flags.
 *
 * @note If @c UseRibbonFrame is enabled, frameless window support
 *       will be added automatically.
 */
MainWindowInternal::MainWindowInternal(const QString& uniqueName,
                                       QWidget* parent,
                                       KDDockWidgets::MainWindowOptions options,
                                       SARibbonMainWindowStyles style,
                                       Qt::WindowFlags flags)
    : KDDockWidgets::QtWidgets::MainWindow(uniqueName, options, parent, flags),
      d{std::make_unique<MainWindowInternal::Impl>(this)} {
    d->ribbonMainWindowStyle = style;
    d->checkMainWindowFlag();
    if (d->isRibbonBarUsed()) {
        if (d->isRibbonFrameUsed()) {  // frameless
            d->installFrameless(this);
        }
        setRibbonBar(createRibbonBar());
        setRibbonTheme(ribbonTheme());
        if (d->isNativeFrameUsed()) {
            // When using the native frame in ribbon mode, the icon will be
            // hidden and the compact mode will be set by default.
            if (SARibbonBar* bar = ribbonBar()) {
                // Hide the icon
                bar->setTitleIconVisible(false);
                // Set to compact mode
                bar->setRibbonStyle(SARibbonBar::RibbonStyleCompactThreeRow);
            }
        }
    }

    connect(qApp, &QApplication::primaryScreenChanged, this,
            &MainWindowInternal::onPrimaryScreenChanged);
}

MainWindowInternal::~MainWindowInternal(){}

#if SARIBBON_USE_3RDPARTY_FRAMELESSHELPER
/**
 * @brief
 * If custom widgets are added to non-clickable areas of the Ribbon
 * (such as the title bar) and need to become clickable, this interface
 * must be called to mark them as clickable.
 *
 * @param w The widget to be marked.
 * @param visible Whether the widget should be treated as clickable.
 */
void MainWindowInternal::setFramelessHitTestVisible(QWidget* w, bool visible) {
    auto fh = d->framelessHelper;
    fh->setHitTestVisible(const_cast<QWidget*>(w), visible);
}
#else
SAFramelessHelper* MainWindowInternal::framelessHelper() const {
    return (d->framelessHelper);
}

void MainWindowInternal::setRubberBandOnResize(bool on) {
    if (SAFramelessHelper* fh = framelessHelper()) {
        fh->setRubberBandOnResize(on);
    }
}
bool MainWindowInternal::isRubberBandOnResize() const {
    if (SAFramelessHelper* fh = framelessHelper()) {
        return fh->rubberBandOnResisze();
    }
    return false;
}
#endif

void MainWindowInternal::updateWindowFlag(Qt::WindowFlags flags) {
    if (d->isRibbonFrameUsed()) {
        if (SARibbonSystemButtonBar* p = d->systemButtonBar) {
            p->updateWindowFlag(flags);
        }
    }
}

SARibbonBar* MainWindowInternal::ribbonBar() const {
    return qobject_cast<SARibbonBar*>(menuWidget());
}

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

SARibbonTheme MainWindowInternal::ribbonTheme() const {
    return d->currentRibbonTheme;
}

/**
 * @brief MainWindowInternal::setRibbonTheme
 *
 * Note: In some versions of Qt, setting the theme in the constructor
 * may not take full effect. You can use QTimer to defer it to the end
 * of the event queue, for example:
 * @code
 * QTimer::singleShot(0, this, [this]() {
 *     this->setRibbonTheme(SARibbonMainWindow::RibbonThemeDark);
 * });
 * @endcode
 *
 * @param theme The ribbon theme to apply.
 */
void MainWindowInternal::setRibbonTheme(SARibbonTheme theme) {
    SA::setBuiltInRibbonTheme(this, theme);
    d->currentRibbonTheme = theme;
    if (SARibbonBar* bar = ribbonBar()) {
        // 1. Spacing of the tabBar
        if (SARibbonTabBar* tab = bar->ribbonTabBar()) {
            MainWindowInternal::Impl::updateTabBarMargins(tab, theme);
        }
        // 2. Update Context Colors
        MainWindowInternal::Impl::updateContextColors(bar, theme);
        // 3. BaseLine Color of the tabBar
        MainWindowInternal::Impl::updateTabBarBaseLineColor(bar, theme);
    }
}

SARibbonSystemButtonBar* MainWindowInternal::systemButtonBar() const {
    return d->systemButtonBar;
}
SARibbonMainWindowStyles MainWindowInternal::ribbonMainwindowStyle() const {
    return d->ribbonMainWindowStyle;
}

bool MainWindowInternal::eventFilter(QObject* obj, QEvent* e) {
#if SARIBBON_USE_3RDPARTY_FRAMELESSHELPER
#else
    // This event filter is used to forward actions from the ribbonBar to the
    // main window, and then on to the frameless helper. Because the ribbonBar
    // can cover the frameless area, the frameless helper cannot capture these
    // events directly. Therefore, ribbonBar()->installEventFilter(this) is
    // required.

    if (obj == ribbonBar()) {
        switch (e->type()) {
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseMove:
            case QEvent::Leave:
            case QEvent::HoverMove:
            case QEvent::MouseButtonDblClick: {
                QApplication::sendEvent(this, e);
            } break;
            default:
                break;
        }
    }
#endif
    return KDDockWidgets::QtWidgets::MainWindow::eventFilter(obj, e);
}

SARibbonBar* MainWindowInternal::createRibbonBar() {
    SARibbonBar* ribbonBar = RibbonSubElementFactory->createRibbonBar(this);
    return ribbonBar;
}

void MainWindowInternal::onPrimaryScreenChanged(QScreen* screen) {
    Q_UNUSED(screen);
    // After switching the primary screen, recalculate all sizes
    if (SARibbonBar* bar = ribbonBar()) {
        qDebug() << "Primary Screen Changed";
        bar->updateRibbonGeometry();
    }
}

MainWindowEventFilter::MainWindowEventFilter(QObject* parent)
    : QObject(parent) {}

MainWindowEventFilter::~MainWindowEventFilter() {}

bool MainWindowEventFilter::eventFilter(QObject* obj, QEvent* e) {
    if (e && obj) {
        if (e->type() == QEvent::Resize) {
            if (MainWindowInternal* p =
                    qobject_cast<MainWindowInternal*>(obj)) {
                if (SARibbonBar* ribbonBar = p->ribbonBar()) {
                    QMargins margin = p->contentsMargins();
                    ribbonBar->setFixedWidth(p->size().width() - margin.left() -
                                             margin.right());
                }
            }
        }
    }
    return QObject::eventFilter(obj, e);
}
