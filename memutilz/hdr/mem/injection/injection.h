#pragma once

#include "mem/Process.h"
#include <windows.h>

namespace mem {
	namespace injection {
		using fp_LoadLibrary =		HMODULE(WINAPI*)(LPCSTR lpLibFileName);
		using fp_GetProcAddress =	FARPROC(WINAPI*)(HMODULE hModule, LPCSTR lpProcName);
		using fp_DLL_ENTRY =		BOOL(WINAPI*)(BYTE* hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

		#ifdef _WIN64
			using fp_RtlAddFunctionTable = BOOLEAN(__cdecl*)(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD64 BaseAddress);
		#endif
		
		enum MM_STATUS {
			MM_PENDING		= 1 << 0,
			MM_SUCCESS		= 1 << 1,
			MM_FAILURE		= 1 << 2,
			MM_NO_SEH_SUPP	= 1 << 3
		};

		typedef struct MM_Data {
			fp_LoadLibrary fpLoadLibrary;
			fp_GetProcAddress fpGetProcAddress;
			
			#ifdef _WIN64
				fp_RtlAddFunctionTable fpRtlAddFunctionTable;
			#endif
			uintptr_t baseAddress;
			MM_STATUS status{ MM_PENDING };
			bool supportSEH{ false };
			DWORD fdwReason;
			LPVOID lpvReserved;
		} MM_DATA, *PMM_DATA;
		
		std::vector<BYTE> readPeFile(std::string_view peFile);
		bool ManualMap(Process& targetProc, std::string_view dllPath, DWORD fdwReason = DLL_PROCESS_ATTACH, LPVOID lpvReserved = nullptr, bool supportSEH = true);
		void __stdcall shellcode(MM_Data* data);
	}
}