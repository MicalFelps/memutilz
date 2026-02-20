#include "MainWindow.h"

MainWindow::MainWindow(const QString& uniqueName, QWidget* parent,
                       bool frameless, KDDockWidgets::MainWindowOptions options,
                       Qt::WindowFlags flags)
    : MainWindowInternal(uniqueName, parent, frameless, options, flags) {}
