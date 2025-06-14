#pragma once

#include "gui/common.h"
#include "ui_MainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();
    void initialize();
private:
    Ui::MainWindowClass* ui;

    std::unique_ptr<mem::Process> proc;
    std::unique_ptr<mem::Meminfo> meminfo;
    std::unique_ptr<mem::Memdump> memdump;

    void printError(const std::wstring& err);
    void printError(const std::string& err);
};