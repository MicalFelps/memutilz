#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <SARibbonMainWindow.h>
#include <SARibbonBar.h>

struct MainWindowPrivate;


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow() override;
protected:
    virtual void closeEvent(QCloseEvent* event) override;
private:
    MainWindowPrivate* d;
    friend struct MainWindowPrivate;
};

#endif