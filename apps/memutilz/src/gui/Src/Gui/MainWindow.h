#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStatusbar>
#include <QCloseEvent>

#include <SARibbonMainWindow.h>

#include "ApplicationWidget.h"
#include "RibbonBar.h"
#include "CentralDockingArea.h"

class MainWindow : public SARibbonMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(
        QWidget* parent = nullptr,
        SARibbonMainWindowStyles style = SARibbonMainWindowStyleFlag::UseRibbonMenuBar | SARibbonMainWindowStyleFlag::UseRibbonFrame,
        const Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~MainWindow() = default;

protected:
    virtual void closeEvent(QCloseEvent* event) override;
private:
    void setupUi();

    ApplicationWidget* _applicationWidget{ nullptr };
    RibbonBar* _ribbonBar{ nullptr };
    CentralDockingArea* _centralDockingArea{ nullptr };
    QStatusBar* _statusBar{ nullptr };
};

#endif