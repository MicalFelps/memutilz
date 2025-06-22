#include "gui/MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new(std::nothrow) gui::Ui_MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::initialize() {
    try {
        proc = std::make_unique<mem::Process>(L"ac_client.exe");

        meminfo = std::make_unique<mem::Meminfo>(proc.get());
        meminfo->find_page_info();

        memdump = std::make_unique<mem::Memdump>(meminfo.get());
        memdump->dump();

        ui->hexview->setMemdump(memdump.get());
    }
    catch (const mem::Exception& e) {
        printError(e.full_msg());
    }
    catch (const std::runtime_error& e) {
        printError(e.what());
    }
    catch (...) {
        printError(L"[!] Unknown Error\n");
    }
}

void MainWindow::printError(const std::wstring& err) {
    QMessageBox::critical(this, "Error", QString::fromStdWString(err));
}

void MainWindow::printError(const std::string& err) {
    QMessageBox::critical(this, "Error", QString::fromStdString(err));
}