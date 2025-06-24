#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gui/common.h"
#include "Ui_MainWindow.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();
    void initialize();
private:
    gui::Ui_MainWindow* ui;

    std::unique_ptr<mem::Process> proc;
    std::unique_ptr<mem::Meminfo> meminfo;
    std::unique_ptr<mem::Memdump> memdump;

    void printError(const std::wstring& err);
    void printError(const std::string& err);
};

#endif