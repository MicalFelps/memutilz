#include <QBoxLayout.h>
#include "MainWindow.h"
#include <MemorySource.h>

struct MainWindowPrivate {
    MainWindow* _this;
    QWidget* _central{ nullptr };
    QHBoxLayout* _layout{ nullptr };

    MainWindowPrivate(MainWindow* _public);
};

MainWindowPrivate::MainWindowPrivate(MainWindow* _public)
    : _this{ _public }
    , _central{ new QWidget(_this) }
    , _layout{ new QHBoxLayout(_central) }
{
}

// ------------------------------------------------------------------

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , d{ new MainWindowPrivate(this) } {

    setMinimumSize(250, 50);
    resize(1600, 900);

    uint8_t code[] = {
        0x48, 0x89, 0xE5,                    // mov rbp, rsp
        0x48, 0x83, 0xEC, 0x30,              // sub rsp, 48
        0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00,  // lea rax, [rip]
        0xC3                                 // ret
    };

    std::string result = basic_zydis_disasm(code, sizeof(code));
    qDebug() << result << "\n";
}
MainWindow::~MainWindow() { delete d; }

void MainWindow::closeEvent(QCloseEvent* event) {
    QMainWindow::closeEvent(event);
}