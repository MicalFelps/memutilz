#include "gui/common.h"
#include "gui/MainWindow.h"
#include "utilz/privilege.h"

/*
    try {
        mem::Process proc{ L"ac_client.exe" };

        proc.set_handle(OpenProcess(PROCESS_ALL_ACCESS, 0, proc.get_pid()));

        mem::Meminfo meminfo(&proc);
        meminfo.get_page_info();

        mem::Memdump memdump(&meminfo);
        memdump.dump();

        Sleep(10000);
    }
    catch (const mem::Exception& e) {
        std::cerr << e.full_msg() << '\n';
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
    }
    catch (...) {
        std::cerr << "[!] Unknown Error\n";
    }
*/

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setStyle("fusion");

    if (!utilz::privilege::CheckAndRequestAdmin())
        return 1; // Exit if we're not admin or if elevation failed / got declined

    MainWindow w;
    w.setWindowTitle("Memutilz 1.0");
    w.show();
    return app.exec();
}