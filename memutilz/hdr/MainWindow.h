#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>

struct MainWindowPrivate;

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    // void createDockWidgets();

protected:
    // later if you want to save the state of anything
    // virtual void closeEvent(QCloseEvent* parent = nullptr) override;
public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow() = default;
public slots:
    //bool openFile(const QString& fileName);
};

#endif