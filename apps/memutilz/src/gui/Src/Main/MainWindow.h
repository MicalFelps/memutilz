#pragma once

#include "MainWindowInternal.h"
#include "../Components/ApplicationWidget.h"
// #include "../Components/DockManager.h"

/**
 * @brief The real MainWindow class that's used to orchestrate connections
 * between widgets
 */
class MainWindow : public MainWindowInternal {
    Q_OBJECT

   public:
    explicit MainWindow(const QString& uniqueName, QWidget* parent = nullptr,
                        KDDockWidgets::MainWindowOptions options = {},
                        SARibbonMainWindowStyles style =
                            SARibbonMainWindowStyleFlag::UseRibbonMenuBar |
                            SARibbonMainWindowStyleFlag::UseRibbonFrame,
                        Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~MainWindow() = default;

   private:
    ApplicationWidget* _applicationWidget;
    // DockManager* _dockManager;
};
