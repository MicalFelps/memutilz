#include <SARibbonButtonGroupWidget.h>
#include <SARibbonQuickAccessBar.h>
#include <SARibbonStackedWidget.h>
#include <SARibbonTabBar.h>
#include <SARibbonTitleIconWidget.h>
#include <SARibbonUtil.h>

#include "../Components/RibbonBar/RibbonElementManager.h"
#include "MainWindowInternal.h"

/**
 * @brief MainWindowInternal Impl class
 */
struct MainWindowInternal::Impl {
    Impl(MainWindowInternal* _public, bool _frameless)
        : _this{_public}, frameless{_frameless} {}

    void installWindowAgent(MainWindowInternal* _public);

    // Theme specific
    static void updateTabBarMargins(SARibbonTabBar* tab, SARibbonTheme theme);
    static void updateContextColors(SARibbonBar* bar, SARibbonTheme theme);
    static void updateTabBarBaseLineColor(SARibbonBar* bar,
                                          SARibbonTheme theme);

    MainWindowEventFilter* eventFilter{nullptr};
    SARibbonTheme currentRibbonTheme{SARibbonTheme::RibbonThemePalette};
    SystemButtonBar* systemButtonBar{nullptr};
    QWK::WidgetWindowAgent* windowAgent{nullptr};
    bool frameless{true};

   private:
    MEMUTILZ_DECLARE_PUBLIC(MainWindowInternal)
};

void MainWindowInternal::Impl::installWindowAgent(MainWindowInternal* _public) {
    windowAgent = new QWK::WidgetWindowAgent(_public);
    windowAgent->setup(_public);
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

MainWindowInternal::MainWindowInternal(const QString& uniqueName,
                                       QWidget* parent, bool frameless,
                                       KDDockWidgets::MainWindowOptions options,
                                       Qt::WindowFlags flags)
    : KDDockWidgets::QtWidgets::MainWindow(uniqueName, options, parent, flags),
      d{std::make_unique<MainWindowInternal::Impl>(this, frameless)} {
    if (frameless) d->installWindowAgent(this);
    setRibbonBar(createRibbonBar());
    setRibbonTheme(ribbonTheme());

    if (!frameless) {
        if (SARibbonBar* bar = ribbonBar()) {
            // Hide the icon
            bar->setTitleIconVisible(false);
            // Set to compact mode
            bar->setRibbonStyle(SARibbonBar::RibbonStyleCompactThreeRow);
        }
    }

    connect(qApp, &QApplication::primaryScreenChanged, this,
            &MainWindowInternal::onPrimaryScreenChanged);
}
MainWindowInternal::~MainWindowInternal() {}

SARibbonBar* MainWindowInternal::ribbonBar() const {
    return qobject_cast<SARibbonBar*>(menuWidget());
}
void MainWindowInternal::setRibbonBar(SARibbonBar* ribbonBar) {
    QWidget* oldMenu = QMainWindow::menuWidget();
    if (oldMenu) oldMenu->deleteLater();

    KDDockWidgets::QtWidgets::MainWindow::setMenuWidget(ribbonBar);
    ribbonBar->setMainWindowStyles(
        SARibbonMainWindowStyleFlag::UseRibbonMenuBar |
        (isFrameless() ? SARibbonMainWindowStyleFlag::UseRibbonFrame
                       : SARibbonMainWindowStyleFlag::UseNativeFrame));

    if (isFrameless()) {
        const int titleBarH = ribbonBar->titleBarHeight();

        if (!d->systemButtonBar) {
            d->systemButtonBar =
                RibbonSubElementFactory->createSystemButtonBar(this);
            d->systemButtonBar->setObjectName(
                QStringLiteral("objSARibbonSystemButtonBar"));
            d->systemButtonBar->setIconSize(QSize(18, 18));

            installEventFilter(d->systemButtonBar);
        }

        SystemButtonBar* systemBar = d->systemButtonBar;
        systemBar->updateSystemButtonStates(windowState());
        systemBar->setWindowTitleHeight(titleBarH);

        // Ensure the sysbar stays on the topmost layer, to prevent it from
        // being covered by the ribbonbar when titleBarHe ribbonbar is set a
        // second time.
        systemBar->raise();
        systemBar->show();

        ribbonBar->titleIconWidget()->setIcon(windowIcon());

        auto agent = d->windowAgent;
        agent->setTitleBar(ribbonBar);
        // The following widgets/windows need to be allowed to receive mouse
        // clicks.
        agent->setHitTestVisible(systemBar);                       // IMPORTANT!
        agent->setHitTestVisible(ribbonBar->ribbonTabBar());       // IMPORTANT!
        agent->setHitTestVisible(ribbonBar->rightButtonGroup());   // IMPORTANT!
        agent->setHitTestVisible(ribbonBar->applicationButton());  // IMPORTANT!
        agent->setHitTestVisible(ribbonBar->quickAccessBar());     // IMPORTANT!
        agent->setHitTestVisible(
            ribbonBar->ribbonStackedWidget());                   // IMPORTANT!
        agent->setHitTestVisible(ribbonBar->titleIconWidget());  // IMPORTANT!

        if (systemBar->closeButton()) {
            agent->setSystemButton(QWK::WindowAgentBase::Close,
                                   systemBar->closeButton());
        }
        if (systemBar->minimizeButton()) {
            agent->setSystemButton(QWK::WindowAgentBase::Minimize,
                                   systemBar->minimizeButton());
        }
        if (systemBar->maximizeButton()) {
            agent->setSystemButton(QWK::WindowAgentBase::Maximize,
                                   systemBar->maximizeButton());
        }
        // Finally, it must be raised; otherwise, newly added elements will be
        // covered.
        if (d->systemButtonBar) d->systemButtonBar->raise();
    } else {
        ribbonBar->setTitleIconVisible(false);
        ribbonBar->setRibbonStyle(SARibbonBar::RibbonStyleCompactThreeRow);
    }

    if (!d->eventFilter) {
        d->eventFilter = new MainWindowEventFilter(this);
        installEventFilter(d->eventFilter);
    }
}

SARibbonTheme MainWindowInternal::ribbonTheme() const {
    return d->currentRibbonTheme;
}
void MainWindowInternal::setRibbonTheme(SARibbonTheme theme) {
    SA::setBuiltInRibbonTheme(this, theme);
    d->currentRibbonTheme = theme;
    if (SARibbonBar* bar = ribbonBar()) {
        // 1. Spacing of the tab bar
        if (SARibbonTabBar* tab = bar->ribbonTabBar()) {
            MainWindowInternal::Impl::updateTabBarMargins(tab, theme);
        }
        // 2. Update Context Colors
        MainWindowInternal::Impl::updateContextColors(bar, theme);
        // 3. BaseLine Color of the tab bar
        MainWindowInternal::Impl::updateTabBarBaseLineColor(bar, theme);
    }
}

SystemButtonBar* MainWindowInternal::systemButtonBar() const {
    return d->systemButtonBar;
}

void MainWindowInternal::updateWindowFlags(Qt::WindowFlags flags) {
    if (isFrameless()) {
        if (SystemButtonBar* sbb = d->systemButtonBar) {
            sbb->setupSystemButtons(flags);
        }
    }
}
bool MainWindowInternal::isFrameless() const { return d->frameless; }

SARibbonBar* MainWindowInternal::createRibbonBar() {
    SARibbonBar* bar = RibbonSubElementFactory->createRibbonBar(this);
    return bar;
}
void MainWindowInternal::onPrimaryScreenChanged(QScreen* screen) {
    Q_UNUSED(screen);
    // After switching the primary screen, recalculate all sizes
    if (SARibbonBar* bar = ribbonBar()) {
        bar->updateRibbonGeometry();
    }
}

// ########################################
// MainWindowEventFilter
// ########################################

MainWindowEventFilter::MainWindowEventFilter(QObject* parent)
    : QObject(parent) {}
MainWindowEventFilter::~MainWindowEventFilter() {}
bool MainWindowEventFilter::eventFilter(QObject* obj, QEvent* e) {
    if (e && obj) {
        if (e->type() == QEvent::Resize) {
            if (MainWindowInternal* p =
                    qobject_cast<MainWindowInternal*>(obj)) {
                if (SARibbonBar* bar = p->ribbonBar()) {
                    QMargins margin = p->contentsMargins();
                    bar->setFixedWidth(p->size().width() - margin.left() -
                                       margin.right());
                }
            }
        }
    }
    return QObject::eventFilter(obj, e);
}
