#include "gui/common.h"
#include "gui/MainWindow.h"
#include "priv/privilege.h"

#include "Process.h"
#include "mem/Memscan.h"
#include "coms/coms.h"

#include <windows.h>
#include <cstdio>
#include <io.h>
#include <fcntl.h>
#include <iostream>

int main(int argc, char** argv)
{
    /*
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
    */
    if (AllocConsole()) {
        FILE* f{};

        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        freopen_s(&f, "CONIN$", "r", stdin);

        try {
            mem::Process process{ L"obsidian.exe" };
            std::cout << "--- [PID] " << process.getPID() << " ---" << '\n' << '\n';

            mem::PageInfo pageinfo{ &process };
            mem::Memdump dumper{ &pageinfo };
            mem::Memscan scanner { &dumper };

            dumper.dump();

            mem::Memscan::ScanResult result = scanner.ScanPattern(mem::Memscan::Pattern::fromString("48 89 5C 24 18 55 48 8B EC 48 83 EC 30 48 8B 05 ?? ?? ?? ?? 48 BB"), mem::Memscan::ScanOptions{});
            std::cout << "Pattern found at:\n";

            for (uintptr_t address : result.addresses) {
                std::cout << std::hex << "  0x" << address << '\n';
            }
            
            std::cin.get();
        }
        catch (mem::Exception& e) {
            std::cerr << e.full_msg() << '\n';
        }
        catch (...) {
            std::cerr << "Unknown Error" << '\n';
        }

        std::cin.get();
        FreeConsole();
    }
    else {
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