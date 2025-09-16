#pragma once

#include "mem/Process.h"
#include <windows.h>

namespace mem {
	namespace injection {
		using fp_LoadLibraryW =		HMODULE(WINAPI*)(LPCWSTR lpLibFileName);
		using fp_GetProcAddress =	FARPROC(WINAPI*)(HMODULE hModule, LPCSTR lpProcName);
		using fp_DLL_ENTRY =		BOOL(WINAPI*)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

		#ifdef _WIN64
			using fp_RtlAddFunctionTable = BOOLEAN(__cdecl*)(PRUNTIME_FUNCTION FunctionTable, DWORD EntryCount, DWORD64 BaseAddress);
		#endif
		
		enum MM_STATUS {
			MM_SUCCESS = 1 << 0,
			MM_FAILURE = 1 << 1,
			MM_PENDING = 1 << 2
		};

		typedef struct MM_Data {
			fp_LoadLibraryW fpLoadLibrary;
			fp_GetProcAddress fpGetProcAddress;
			
			#ifdef _WIN64
				fp_RtlAddFunctionTable fpRtlAddFunctionTable;
			#endif
			uintptr_t baseAddress;
			MM_STATUS status{ MM_PENDING };
			bool supportSEH{ false };
		} MM_DATA, *PMM_DATA;
		
		std::vector<BYTE> readPeFile(std::string_view peFile);
		bool ManualMap(Process& targetProc, std::string_view dllPath, bool supportSEH);
		void __stdcall shellcode(MM_Data* data);
	}
}