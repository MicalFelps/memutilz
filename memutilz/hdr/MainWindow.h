#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>

#include "SideBar/SideBar.h"

struct MainWindowPrivate;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow() = default;

public slots:
    //bool openFile(const QString& fileName);

protected:
    // later if you want to save the state of anything
    // virtual void closeEvent(QCloseEvent* parent = nullptr) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    SideBar* _sidebar{ nullptr };
    QWidget* _content{ nullptr };
    // void createDockWidgets();
};

#endif