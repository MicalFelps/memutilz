#include "gui/common.h"
#include "gui/MainWindow.h"
#include "priv/privilege.h"

#include "Process.h"
#include "mem/constants.h"
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
            using ScanResult = mem::Memscan::ScanResult;
            using Pattern = mem::Memscan::Pattern;
            using ScanOptions = mem::Memscan::ScanOptions;
            
            mem::Process process{ L"obsidian.exe" };
            std::cout << "--- [PID] " << process.getPID() << " ---" << '\n' << '\n';

            mem::PageInfo pageinfo{ &process };
            mem::Memdump dumper{ &pageinfo };
            mem::Memscan scanner { &dumper };

            dumper.dump();

            // 48 83 EC 28 E8 ?? ?? ?? ?? 48 83 C4 28
            ScanOptions opt{
                mem::PAGE_READ_FLAGS,
                1,
                true,
            };
            ScanResult result = scanner.ScanPattern(Pattern::fromString("\x48\x83\xEC\x28\xE8\xFF\xFF\xFF\xFF\x48\x83\xC4\x28", "xxxxx????xxxx"), opt);
            // ScanResult result = scanner.ScanPattern(Pattern::fromString("48 83 EC 28 E8 ?? ?? ?? ?? 48 83 C4 28"), opt);
            // ScanResult result = scanner.ScanValue<uintptr_t>(5, opt);
            std::cout << "Pattern found at:\n";

            for (uintptr_t address : result.addresses) {
                std::cout << std::hex << "  0x" << address << '\n';
            }
            
            std::cin.get();
        }
        catch (std::invalid_argument& e) {
            std::cerr << e.what() << '\n';
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