#pragma once

#include <QStatusBar>
#include <QCloseEvent>

#include <SARibbonMainWindow.h>

#include "../Components/ApplicationWidget.h"
#include "../Components/RibbonBar.h"
#include "../Components/CentralDockingArea.h"

struct MainWindowPrivate;

class MainWindow : public SARibbonMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(
        QWidget* parent = nullptr,
        SARibbonMainWindowStyles style = SARibbonMainWindowStyleFlag::UseRibbonMenuBar | SARibbonMainWindowStyleFlag::UseRibbonFrame,
        const Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~MainWindow() override;

protected:
    virtual void closeEvent(QCloseEvent* event) override;
private:
    friend struct MainWindowPrivate;
    MainWindowPrivate* d;

    ApplicationWidget* _applicationWidget{ nullptr };
    RibbonBar* _ribbonBar{ nullptr };
    CentralDockingArea* _centralDockingArea{ nullptr };
    QStatusBar* _statusBar{ nullptr };

    void setupUi();
    void handleWindowSignal();
};
