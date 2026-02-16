#pragma once

#include <QScreen>
#include <SARibbonBar.h>
#include <SARibbonSystemButtonBar.h>
#include <kddockwidgets/MainWindow.h>

#if !SARIBBON_USE_3RDPARTY_FRAMELESSHELPER
#include <SAFramelessHelper.h>  // fallback just in case
#endif

#include "../../../MemutilzMacros.h"

/**
 * @brief Private MainWindow class to handle ribbon integration support,
 * frameless implementation, and event filtering built on top of KDDockWidgets's
 * mainWindow, the actual MainWindow class inherits from this.
 */
class MainWindowInternal : public KDDockWidgets::QtWidgets::MainWindow {
    Q_OBJECT

   public:
    explicit MainWindowInternal(
        const QString& uniqueName, QWidget* parent = nullptr,
        KDDockWidgets::MainWindowOptions options = {},
        SARibbonMainWindowStyles style =
            SARibbonMainWindowStyleFlag::UseRibbonMenuBar |
            SARibbonMainWindowStyleFlag::UseRibbonFrame,
        Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~MainWindowInternal() = default;
    bool isRibbonBar() const { return (nullptr != ribbonBar()); }

#if SARIBBON_USE_3RDPARTY_FRAMELESSHELPER
    // If a custom widget is added to the title bar or other non-clickable areas
    // in the ribbon, and you want it to be clickable, call this function to
    // indicate it should respond to clicks
    void setFramelessHitTestVisible(QWidget* w, bool visible = true);
#else
    SAFramelessHelper* framelessHelper() const;

    // Use lazy resizing (with a rubber band), works nicer with
    // widgets that do heavy rendering
    void setRubberBandOnResize(bool on);
    bool isRubberBandOnResize() const;
#endif

    // This function is only used to control the visibility of the minimize,
    // maximize, and close buttons, really useful for tiling WMs
    void updateWindowFlag(Qt::WindowFlags flags);

    // --- Getters & Setters

    SARibbonBar* ribbonBar() const;
    void setRibbonBar(SARibbonBar* ribbonBar);

    SARibbonTheme ribbonTheme() const;
    void setRibbonTheme(SARibbonTheme theme);

    // Get the bar that contains the maximize, minimize, and close buttons.
    // You can use this function to place additional content next to those
    // buttons.
    SARibbonSystemButtonBar* systemButtonBar() const;
    // Get the current style of the main window
    SARibbonMainWindowStyles ribbonMainwindowStyle() const;

   protected:
    // Forward ribbonBar's events to the frameless window handler
    virtual bool eventFilter(QObject* obj, QEvent* e) override;

    // Factory function for creating the ribbon bar
    SARibbonBar* createRibbonBar();
   private slots:
    /**
     * @brief Updates the ribbonBar's geometry
     *
     * When the primary screen changes, that might change
     * - DPI (scaling factor)
     * - Screen resolution
     * - Available geometry
     * - device pixel ratio
     * So we update our UI
     */
    void onPrimaryScreenChanged(QScreen* screen);

   private:
    MEMUTILZ_DECLARE_PRIVATE()
    friend class SARibbonBar;
};
