#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QMenuBar.h>
#include "MemoryView/AssemblyView.h"
#include "MemoryView/Hexview.h"

namespace gui {
    class Ui_MainWindow {
    public:
        QWidget* centralWidget;
        gui::Hexview* hexview;
        gui::AssemblyView* assemblyView;
        QMenuBar* menuBar;

        void setupUi(QMainWindow* MainWindow)
        {
            MainWindow->resize(1288, 881);
            if (MainWindow->objectName().isEmpty())
                MainWindow->setObjectName("MainWindow");

            centralWidget = new QWidget{ MainWindow };
            MainWindow->setCentralWidget(centralWidget);

            // --- Menu Bar ---
            menuBar = new QMenuBar(MainWindow);
            menuBar->setGeometry(QRect(0, 0, 1288, 21));
            menuBar->setAutoFillBackground(true);
            menuBar->setNativeMenuBar(true);

            // Actions
            QAction* action_Attach_to_Process = new QAction("❤️ Attach to Process", MainWindow);
            QAction* action_Re_Attach_to_Process = new QAction("💚 Re-Attach to Process", MainWindow);
            QAction* action_Detach = new QAction("💔 Detach", MainWindow);
            QAction* action_Save_Dump = new QAction("📷 Save Dump", MainWindow);
            QAction* action_Load_Dump = new QAction("💾 Load Dump", MainWindow);
            QAction* action_Quit = new QAction("⛔ Quit", MainWindow);
            QAction* action_Modules = new QAction("📦 Modules", MainWindow);
            QAction* action_Threads = new QAction("🧵 Threads", MainWindow);
            QAction* action_Memory_Map = new QAction("🧩 Memory Map", MainWindow);
            QAction* action_Inject_DLL = new QAction("💉 Inject DLL", MainWindow);
            QAction* action_Scanner = new QAction("🔎 Scanner", MainWindow);
            QAction* actionAbout = new QAction("About", MainWindow);

            // Menus
            QMenu* menuFile = new QMenu("File", menuBar);
            menuFile->addAction(action_Attach_to_Process);
            menuFile->addAction(action_Re_Attach_to_Process);
            menuFile->addAction(action_Detach);
            menuFile->addSeparator();
            menuFile->addAction(action_Save_Dump);
            menuFile->addAction(action_Load_Dump);
            menuFile->addSeparator();
            menuFile->addAction(action_Quit);

            QMenu* menuView = new QMenu("View", menuBar);
            menuView->addAction(action_Modules);
            menuView->addAction(action_Threads);
            menuView->addAction(action_Memory_Map);

            QMenu* menuDebug = new QMenu("Debug", menuBar);

            QMenu* menuTools = new QMenu("Tools", menuBar);
            menuTools->addAction(action_Inject_DLL);
            menuTools->addAction(action_Scanner);

            QMenu* menuHelp = new QMenu("Help", menuBar);
            menuHelp->addAction(actionAbout);

            // Add Menus to MenuBar
            menuBar->addMenu(menuFile);
            menuBar->addMenu(menuView);
            menuBar->addMenu(menuDebug);
            menuBar->addMenu(menuTools);
            menuBar->addMenu(menuHelp);
        } // setupUi
    };
}

#endif