#pragma once

#include <kddockwidgets/MainWindow.h>
#include <QWKWidgets/widgetwindowagent.h>
#include <SARibbonBar.h>

#include "../Components/RibbonBar/SystemButtonBar.h"

#include "../../Globals.h"

/**
 * @brief The actual MainWindow inherits from this class.
 * It handles ribbon integration and frameless support on top
 * of KDDW's MainWindow class.
 */
class MainWindowInternal : public KDDockWidgets::QtWidgets::MainWindow {
    Q_OBJECT

   public:
    explicit MainWindowInternal(const QString& uniqueName,
                                QWidget* parent = nullptr,
                                bool frameless = true,
                                KDDockWidgets::MainWindowOptions options = {},
                                Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~MainWindowInternal() override;

    SARibbonBar* ribbonBar() const;
    void setRibbonBar(SARibbonBar* ribbonBar);

    SARibbonTheme ribbonTheme() const;
    void setRibbonTheme(SARibbonTheme theme);

    SystemButtonBar* systemButtonBar() const;

    void updateWindowFlags(Qt::WindowFlags flags);
    bool isFrameless() const;

   protected:
    // Factory method
    virtual SARibbonBar* createRibbonBar();

   private slots:
    void onPrimaryScreenChanged(QScreen* screen);

   private:
    MEMUTILZ_DECLARE_PRIVATE()
};

/**
 * @brief Event handler for MainWindowInternal, mainly responsible for adjusting
 * the width of the ribbon bar by accounting for margins
 */
class MainWindowEventFilter : public QObject {
    Q_OBJECT
   public:
    explicit MainWindowEventFilter(QObject* parent);
    ~MainWindowEventFilter();
    virtual bool eventFilter(QObject* obj, QEvent* e) override;
};
