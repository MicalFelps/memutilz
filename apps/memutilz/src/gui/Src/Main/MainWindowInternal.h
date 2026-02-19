#pragma once

#include <SARibbonBar.h>

#include <kddockwidgets/MainWindow.h>
#include <QWKWidgets/WidgetWindowAgent.h>

#include "../../../MemutilzMacros.h"

/**
 * @brief The actual MainWindow inherits from this class.
 * It handles ribbon integration and frameless support on top
 * of KDDW's MainWindow class.
 */
class MainWindowInternal : public KDDockWidgets::QtWidgets::MainWindow {
    Q_OBJECT
    
public:
    explicit MainWindowInternal(
        const QString& uniqueName,
        QWidget* parent = nullptr,
        bool frameless = true,
        KDDockWidgets::MainWindowOptions options = {},
        Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~MainWindowInternal() override;

    SARibbonBar* ribbonBar() const;
    void setRibbonBar(SARibbonBar* ribbonBar);

    SARibbonTheme ribbonTheme() const;
    void setRibbonTheme(SARibbonTheme theme);

    SARibbonSystemButtonBar* systemButtonBar() const;

    bool isFrameless() const { return _frameless; }

protected:
    // Factory method
    virtual SARibbonBar* createRibbonBar() = 0;

private slots:
    void onPrimaryScreenChanged(QScreen* screen);

private:
    MEMUTILZ_DECLARE_PRIVATE(MainWindowInternal)
    bool _frameless;
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