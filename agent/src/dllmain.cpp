// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <Windows.h>
#include <iostream>
#include <MinHook.h>

#include "coms/coms.h"

typedef LPVOID (WINAPI* tVirtualAllocEx)(
    HANDLE hProcess,
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD  flAllocationType,
    DWORD  flProtect
);

static tVirtualAllocEx oVirtualAllocEx = nullptr;

LPVOID WINAPI hkVirtualAllocEx(
    HANDLE hProcess,
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD  flAllocationType,
    DWORD  flProtect ) {

    std::cout << "[Call] hkVirtualAllocEx(\n" 
        << "  hProcess: "         << hProcess          << ",\n"
        << "  lpAddress: "        << lpAddress         << ",\n"
        << "  dwSize: "           << dwSize            << ",\n"
        << "  flAllocationType: " << flAllocationType  << ",\n"
        << "  flProtect: "        << flProtect         << "\n"
        << ");" << '\n';

    if (oVirtualAllocEx) {
        return oVirtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
    } else { return nullptr; }
}

// -----------------------------------------------------------------

typedef LPVOID (WINAPI* tVirtualAlloc)(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD  flAllocationType,
    DWORD  flProtect
);

static tVirtualAlloc oVirtualAlloc = nullptr;

LPVOID WINAPI hkVirtualAlloc(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD  flAllocationType,
    DWORD  flProtect) {

    std::cout << "[Call] hkVirtualAlloc(\n"
        << "  lpAddress: "          << "\t\t\t" << lpAddress            << ",\n"
        << "  dwSize: "             << "\t\t\t" << dwSize               << ",\n"
        << "  flAllocationType: "   << "\t\t" << flAllocationType     << ",\n"
        << "  flProtect: "          << "\t\t\t" << flProtect            << "\n"
        << ");" << '\n';

    if (oVirtualAlloc) {
        return oVirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
    }
    else { return nullptr; }
}

// -----------------------------------------------------------------

DWORD WINAPI Main(HMODULE hMod) {
    HANDLE hPipe = CreateFile(
        L"\\\\.\\pipe\\coms",
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to connect to pipe...\n";
        return -1;
    }

    DWORD other_pid;
    DWORD bytesRead;
    if (ReadFile(hPipe, &other_pid, sizeof(DWORD), &bytesRead, nullptr)) {
        if (AttachConsole(other_pid)) {
            FILE* pFile = nullptr;
            freopen_s(&pFile, "CONOUT$", "w", stdout);
            freopen_s(&pFile, "CONOUT$", "w", stderr);
            freopen_s(&pFile, "CONIN$", "r", stdin);

            //std::ios::sync_with_stdio(true);
        } else { return -1; }
    }

    if (MH_Initialize() != MH_OK) {
        std::cerr << "[!] MH_Initialize Failed...\n";
        std::cin.get();

        return -1;
    }

    if (MH_CreateHookApi(L"kernel32", "VirtualAlloc", &hkVirtualAlloc,
        reinterpret_cast<LPVOID*>(&oVirtualAlloc)) != MH_OK) {
        std::cerr << "[!] MH_CreateHookApi Failed...\n";
        MH_Uninitialize();
        FreeConsole();

        std::cin.get();
        return -1;
    }

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        std::cerr << "[!] MH_EnableHook Failed...\n";
        MH_Uninitialize();
        FreeConsole();

        std::cin.get();
        return -1;
    }

    std::cout << "   Press 'Enter' to quit...\n";

    std::cin.get();

    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(&hkVirtualAlloc);
    MH_Uninitialize();

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        HANDLE hThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), hModule, 0, nullptr);
        if (hThread) { CloseHandle(hThread); }
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

