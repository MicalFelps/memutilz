#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gui/common.h"
#include "Ui_MainWindow.h"

struct MainWindowPrivate;

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    friend struct MainWindowPrivate;
    MainWindowPrivate* d; //< pImpl

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    virtual ~MainWindow();

private:
    void printError(const std::wstring& err);
    void printError(const std::string& err);
};

#endif