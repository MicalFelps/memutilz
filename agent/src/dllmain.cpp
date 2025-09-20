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

    std::cout << "[Call] hkVirtualAllocEx(dwSize: " << dwSize << ");" << '\n';

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

    std::cout << "[Call] hkVirtualAlloc(dwSize: " << dwSize << ");" << '\n';

    if (oVirtualAlloc) {
        return oVirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
    }
    else { return nullptr; }
}

// -----------------------------------------------------------------

DWORD WINAPI Main(HMODULE hMod) {
    if (AllocConsole()) {
        FILE* f{};

        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        freopen_s(&f, "CONIN$", "r", stdin);
    }
    std::cout << "We've snuck inside the process!\n";

    if (MH_Initialize() != MH_OK) {
        std::cerr << "[!] MH_Initialize Failed...\n";\
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

    std::cin.get();

    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(&hkVirtualAlloc);
    MH_Uninitialize();
    FreeConsole();

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

