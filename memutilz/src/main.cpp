#include "gui/common.h"
#include "gui/MainWindow.h"
#include "priv/privilege.h"

#include "Process.h"
#include "coms/coms.h"

#include <windows.h>
#include <cstdio>
#include <io.h>
#include <fcntl.h>
#include <iostream>

int main(int argc, char** argv)
{
    if (AllocConsole()) {
        FILE* f{};

        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        freopen_s(&f, "CONIN$", "r", stdin);

        try {
            mem::Process process{ L"obsidian.exe" };
            std::cout << "--- [PID] " << process.getPID() << " ---" << '\n';

            coms::InitializePipeComms(process);
        }
        catch (mem::Exception& e) {
            std::cerr << e.full_msg() << '\n';
        }
        catch (...) {
            std::cerr << "Unknown Error" << '\n';
        }

        std::cin.get();
        FreeConsole();
    } else {
        MessageBox(NULL, L"Failed to alloc console...", L"Error", MB_ICONERROR);
    }

    /*
    QApplication app(argc, argv);
    app.setApplicationName("Memutilz 1.0");
    app.setStyle("fusion");

    if (!priv::CheckAndRequestAdmin())
        return 1; // Exit if we're not admin or if elevation failed / got declined

    MainWindow w;
    w.show();
    return app.exec();
    */
}