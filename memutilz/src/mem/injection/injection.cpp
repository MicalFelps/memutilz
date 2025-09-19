#include "../mem/injection/injection.h"
#include "../mem/Exception.h"
#include <filesystem>
#include <fstream>
#include <windows.h>

namespace fs = std::filesystem;

namespace mem {
	namespace injection {
		std::vector<BYTE> mem::injection::readPeFile(std::string_view peFile)
		{
			if (!fs::exists(peFile)) {
				std::cerr << "File does not exist!" << '\n';
				return std::vector<BYTE>();
			}

			std::ifstream file{ peFile.data(), std::ios::binary };
			if (file.fail()) {
				std::cerr << "Failed to open file!\n";
				return std::vector<BYTE>();
			}

			std::vector<BYTE> buffer{
				(std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>()
			};

			if(buffer.size() < 0x1000) {
				std::cerr << "File size is too small\n";
				return std::vector<BYTE>();
			}

			if (reinterpret_cast<IMAGE_DOS_HEADER*>(buffer.data())->e_magic != 0x5A4D) {
				std::cerr << "File is not a DLL\n";
				return std::vector<BYTE>();
			}

			/*
			size_t bytesPerLine = 16;
			for (int i = 0; i < 0x1000 && i < buffer.size(); i += bytesPerLine) {
				for (int j = 0; j < bytesPerLine && i + j < buffer.size(); ++j) {
					std::cout << std::hex << std::setw(2) << std::setfill('0') <<
						static_cast<int>(buffer[i + j]) << ' ';

				}

				std::cout << "   ";

				for (int j = 0; j < bytesPerLine && i + j < buffer.size(); ++j) {
					char c = static_cast<char>(buffer[i + j]);
					std::cout << (std::isprint(static_cast<BYTE>(c)) ? c : '.');
				}

				std::cout << '\n';
			}
			*/

			return buffer;
		}

		bool ManualMap(Process& targetProc, std::string_view dllPath, DWORD fdwReason, LPVOID lpvReserved, bool supportSEH) {
			auto fileBuffer = readPeFile(dllPath);
			if (fileBuffer.size() == 0) {
				return false;
			}

			BYTE* pBaseAddress = nullptr;

			auto pOldNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(fileBuffer.data() + reinterpret_cast<IMAGE_DOS_HEADER*>(fileBuffer.data())->e_lfanew);
			auto pOldFileHeader = &pOldNtHeader->FileHeader;
			auto pOldOptHeader = &pOldNtHeader->OptionalHeader;

			// Here I'm assuming we're x64, if not we can check that the DLL is valid for that
			if (targetProc.is32Bit() && !(pOldFileHeader->Machine == IMAGE_FILE_MACHINE_I386)) {
				std::cerr << "Can't inject an x64 DLL into a WoW process\n";
				return false;
			}

			std::cout << "[!] Loading File\n";

			Handle hProc{ OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProc.getPID()) };
			pBaseAddress = reinterpret_cast<BYTE*>(VirtualAllocEx(hProc.get(), nullptr, pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

			if (!pBaseAddress) {
				throw mem::Exception("Failed to allocate memory for DLL in target process");
			}

			std::cout << std::hex << "[!] Base Address: 0x" << reinterpret_cast<uintptr_t>(pBaseAddress) << '\n';

			// Write the PE Headers
			if (!WriteProcessMemory(hProc.get(), pBaseAddress, fileBuffer.data(), 0x1000, nullptr)) {
				VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
				throw mem::Exception("Failed to write PE Headers at base address");
			}

			std::cout << "[+] Mapped PE Headers to the first 0x1000 bytes\n";

			// Write the sections
			IMAGE_SECTION_HEADER* pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
			for (size_t i = 0; i < pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
				if (pSectionHeader->SizeOfRawData != 0) {
					if (!WriteProcessMemory(hProc.get(), pBaseAddress + pSectionHeader->VirtualAddress, fileBuffer.data() + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr)) {
						VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
						throw mem::Exception("Failed to map sections to target process\n");
					}
					std::cout << '[' << i << ']' << ' ' << "Mapped: " << pSectionHeader->Name << " @ 0x" << reinterpret_cast<uintptr_t>(pBaseAddress) + pSectionHeader->VirtualAddress << '\n';
				}
			}

			// Write the address of relevant functions for fixing IAT
			// This assumes that the function pointers will be at the same address
			// which is a relatively safe bet to make, since windows works hard to
			// map relevant DLLs like kernel32.dll to the same base address.
			// However there is no guarantee of this working, so it might be better
			// to simply walk the PEB and resolving the address via the export table by
			// indexing instead
			MM_DATA data{};
			data.fpLoadLibrary = LoadLibraryA;
			data.fpGetProcAddress = GetProcAddress;
#ifdef WIN64
			data.fpRtlAddFunctionTable = (fp_RtlAddFunctionTable)RtlAddFunctionTable;
#else
			supportSEH = false;
#endif
			data.baseAddress = reinterpret_cast<uintptr_t>(pBaseAddress);
			data.supportSEH = supportSEH;
			data.fdwReason = fdwReason;
			data.lpvReserved = lpvReserved;

			BYTE* mmDataLoc = reinterpret_cast<BYTE*>(VirtualAllocEx(hProc.get(), nullptr, sizeof(MM_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
			if (!mmDataLoc) {
				VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
				throw mem::Exception("Failed to allocate memory for MM_DATA");
			}

			if (!WriteProcessMemory(hProc.get(), mmDataLoc, reinterpret_cast<LPCVOID>(&data), sizeof(MM_DATA), nullptr)) {
				VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
				VirtualFreeEx(hProc.get(), mmDataLoc, 0, MEM_RELEASE);
				throw mem::Exception("Failed to write MM_DATA");
			}

			std::cout << "[!] Manual Mapping Data at: 0x" << std::hex << reinterpret_cast<uintptr_t>(mmDataLoc) << '\n';

			// Writing the shellcode
			LPVOID shellcodeLoc = VirtualAllocEx(hProc.get(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (!shellcodeLoc) {
				VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
				VirtualFreeEx(hProc.get(), mmDataLoc, 0, MEM_RELEASE);
				throw mem::Exception("Failed to allocate memory for shellcode");
			}

			std::cout << "[!] Shellcode at : 0x" << std::hex << reinterpret_cast<uintptr_t>(shellcodeLoc) << '\n';
			std::cout << "[!] .reload /i memutilz=" << std::hex << reinterpret_cast<uintptr_t>(shellcodeLoc) - 0x899D0 << ",1B96000" << '\n';

			// MSVC on x64 uses jump stubs
			LPCVOID shellcodeAddr{ reinterpret_cast<LPCVOID>(shellcode) };
			auto pShellcode = reinterpret_cast<BYTE*>(shellcode);
			if (pShellcode[0] == 0xE9) {
				auto pRelativeOffset = reinterpret_cast<uint32_t*>(pShellcode+=1);
				shellcodeAddr = reinterpret_cast<LPCVOID>(reinterpret_cast<uintptr_t>(shellcode) + *pRelativeOffset + 0x5);
			}

			if (!WriteProcessMemory(hProc.get(), shellcodeLoc, shellcodeAddr, 0x1000, nullptr)) {
				VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
				VirtualFreeEx(hProc.get(), mmDataLoc, 0, MEM_RELEASE);
				VirtualFreeEx(hProc.get(), shellcodeLoc, 0, MEM_RELEASE);
				throw mem::Exception("Failed to map shellcode to target process");
			}

			std::cout << "	SETUP DONE	" << '\n';
#ifdef _DEBUG
			std::cout << "Press 'Enter' to continue...\n";
			std::cin.get();
#endif
			std::cout << "[!!] Creating Remote Thread" << '\n';
			{
				Handle hRemote{ CreateRemoteThread(hProc.get(), nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(shellcodeLoc), mmDataLoc, 0, nullptr) };
				if (!hRemote) {
					VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
					VirtualFreeEx(hProc.get(), mmDataLoc, 0, MEM_RELEASE);
					VirtualFreeEx(hProc.get(), shellcodeLoc, 0, MEM_RELEASE);
					throw mem::Exception("Failed to create remote thread in target process");
				}
			}

			std::cout << "[+] Thread Created Successfully!" << '\n' << '\n';

			MM_STATUS checkFlag = MM_PENDING;
			while (checkFlag & MM_PENDING) {
				DWORD exitCode = 0;
				GetExitCodeProcess(hProc.get(), &exitCode);
				if (exitCode != STILL_ACTIVE) {
					throw mem::Exception("Process Crashed :(");
				}
				MM_DATA mmDataStatus{};
				ReadProcessMemory(hProc.get(), mmDataLoc, &mmDataStatus, sizeof(MM_DATA), nullptr);
				checkFlag = mmDataStatus.status;

				if (checkFlag & MM_FAILURE) {
					VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
					VirtualFreeEx(hProc.get(), mmDataLoc, 0, MEM_RELEASE);
					VirtualFreeEx(hProc.get(), shellcodeLoc, 0, MEM_RELEASE);
					throw mem::Exception("Shellcode Failed :(");
				} else if (checkFlag & MM_NO_SEH_SUPP) {
					std::cout << "[-] Warning: SEH Support Failed\n";
				}

				Sleep(50);
			}

			// Clear PE Headers
			std::vector<BYTE> nullBuffer(0x1000 * 0x10, 0x0);
			if (!WriteProcessMemory(hProc.get(), pBaseAddress, nullBuffer.data(), 0x1000, nullptr)) {
				throw mem::Exception("Failed to clear PE Headers");
			}

			std::cout << "[-] Cleared PE Headers\n";

			// Clear Un-needed sections
			pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
			for (size_t i = 0; i < pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
				if (pSectionHeader->Misc.VirtualSize) {
					if((!supportSEH && strcmp((char*)pSectionHeader->Name, ".pdata") == 0) ||
						strcmp((char*)pSectionHeader->Name, ".rsrc") == 0 ||
						strcmp((char*)pSectionHeader->Name, ".reloc") == 0 ){

						std::cout << "[-]" << ' ' << "Clearing: " << pSectionHeader->Name << '\n';
						if (!WriteProcessMemory(hProc.get(), pBaseAddress + pSectionHeader->VirtualAddress, nullBuffer.data(), pSectionHeader->SizeOfRawData, nullptr)) {
							throw mem::Exception("Failed to clear section during manual mapping");
						}
					}
				}
			}

			// Change Protection Flags
			pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
			for (size_t i = 0; i < pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
				if (pSectionHeader->Misc.VirtualSize) {
					DWORD old;
					DWORD current = PAGE_READONLY;

					if((pSectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE) > 0){
						current = PAGE_READWRITE;
					} else if ((pSectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) > 0) {
						current = PAGE_EXECUTE_READ;
					}

					if (!VirtualProtectEx(hProc.get(), pBaseAddress + pSectionHeader->VirtualAddress, pSectionHeader->Misc.VirtualSize, current, &old)) {
						throw mem::Exception("Failed to change memory protection");
					}
				}
			}
			DWORD old;
			VirtualProtectEx(hProc.get(), pBaseAddress, IMAGE_FIRST_SECTION(pOldNtHeader)->VirtualAddress, PAGE_READONLY, &old);

			if (!WriteProcessMemory(hProc.get(), shellcodeLoc, nullBuffer.data(), 0x1000, nullptr)) {
				std::cerr << "[-] Warning: Failed to clear shellcode\n";
			}
			if (!VirtualFreeEx(hProc.get(), shellcodeLoc, 0, MEM_RELEASE)) {
				std::cerr << "[-] Warning: Failed to free shellcode memory\n";
			}
			if (!VirtualFreeEx(hProc.get(), mmDataLoc, 0, MEM_RELEASE)) {
				std::cerr << "[-] Warning: Failed to free MM_DATA memory\n";
			}

			return true;
		}

#define RELOC_FLAG32(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else
#define RELOC_FLAG RELOC_FLAG32
#endif

		// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#special-sections

		#pragma optimize("", off)
		#pragma runtime_checks("", off)

		void __stdcall shellcode(MM_Data* data) {
			BYTE* pBaseAddress = reinterpret_cast<BYTE*>(data->baseAddress);
			auto pOldNtHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(pBaseAddress + reinterpret_cast<IMAGE_DOS_HEADER*>((uintptr_t)pBaseAddress)->e_lfanew);
			auto pOldOptHeader = &pOldNtHeaders->OptionalHeader;

			auto fpLoadLibrary = data->fpLoadLibrary;
			auto fpGetProcAddress = data->fpGetProcAddress;
#ifdef WIN64
			auto fpRtlAddFunctionTable = data->fpRtlAddFunctionTable;
#endif
			auto DllMain = reinterpret_cast<fp_DLL_ENTRY>(pBaseAddress + pOldOptHeader->AddressOfEntryPoint);

			/*
			Sanity check the addresses of LoadLibrary and GetProcAddress with pattern scanning
			by walking the PEB and finding the export table of kernel32.dll
			If they're hooked, simply exit and set MM_FAILURE
			*/

			// Handle Relocation
			BYTE* baseDelta = pBaseAddress - pOldOptHeader->ImageBase;
			if (baseDelta) {
				if (pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size != 0) {
					auto pRelocSection =	reinterpret_cast<IMAGE_BASE_RELOCATION*>(pBaseAddress + pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
					auto pRelocSectionEnd = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<uintptr_t>(pRelocSection) + pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);

					while (pRelocSection < pRelocSectionEnd && pRelocSection->SizeOfBlock != 0) {
						size_t numberOfEntries = ((pRelocSection->SizeOfBlock) - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
						WORD* pEntry = reinterpret_cast<WORD*>(pRelocSection + 1);

						for (size_t i = 0; i < numberOfEntries; ++i, ++pEntry) {
							if (RELOC_FLAG(*pEntry)) {
								uintptr_t* pAddressToPatch = reinterpret_cast<uintptr_t*>(pBaseAddress + pRelocSection->VirtualAddress + ((*pEntry) & 0xFFF));
								*pAddressToPatch += reinterpret_cast<uintptr_t>(baseDelta);
							}
						}
						pRelocSection = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(pRelocSection) + pRelocSection->SizeOfBlock);
					}
				}
			}

			// Handle imports
			if (pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size != 0) {
				auto pImportDirTable = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(pBaseAddress + pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
				while (pImportDirTable->Name) {
					LPCSTR lpLibFileName = reinterpret_cast<LPCSTR>(pBaseAddress + pImportDirTable->Name);
					HINSTANCE hDll = fpLoadLibrary(lpLibFileName); // not sneaky :(

					if (!hDll) {
						data->status = MM_FAILURE;
						return;
					}

					/*
					OriginalFirstThunk points to the import lookup table, which is an array or RVAs or ordinals.
					FirstThunk is the import address table, it's an exact copy of the OriginalFirstThunk,
					before it's values get overwritten with actual pointers.
					*/

					auto pOrigThunk =	reinterpret_cast<uintptr_t*>(pBaseAddress + pImportDirTable->OriginalFirstThunk);
					auto pFirstThunk =	reinterpret_cast<uintptr_t*>(pBaseAddress + pImportDirTable->FirstThunk);

					if (!pImportDirTable->OriginalFirstThunk) {
						pOrigThunk = pFirstThunk;
					}

					for (; *pOrigThunk; ++pOrigThunk, ++pFirstThunk) {
						if (IMAGE_SNAP_BY_ORDINAL(*pOrigThunk)) {
							*pFirstThunk = (uintptr_t)fpGetProcAddress(hDll, reinterpret_cast<LPCSTR>(*pOrigThunk & 0xFFFF));
						} else { // Get by name
							auto pHintNameTable = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(pBaseAddress + (*pOrigThunk));
							*pFirstThunk = (uintptr_t)fpGetProcAddress(hDll, reinterpret_cast<LPCSTR>(pHintNameTable->Name));
						}
					}
					++pImportDirTable;
				}

			}

			// Handle TSL Callbacks
			if(pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size != 0) {
				auto pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(pBaseAddress + pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
				auto fpCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
				for (; fpCallback && *fpCallback; ++fpCallback) {
					(*fpCallback)(pBaseAddress, DLL_PROCESS_ATTACH, nullptr);
				}
			}

			// SEH Support

			bool bExceptionSupportFailure = false;
#ifdef WIN64
			if (data->supportSEH) {
				auto exceptionSection = pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
				if (exceptionSection.Size != 0) {
					if (!fpRtlAddFunctionTable(reinterpret_cast<IMAGE_RUNTIME_FUNCTION_ENTRY*>(pBaseAddress + exceptionSection.VirtualAddress),
						exceptionSection.Size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY),
						(uintptr_t)pBaseAddress)) {

						bExceptionSupportFailure = true;
					}
				}
			}
#endif
			if (bExceptionSupportFailure) {
				data->status = MM_NO_SEH_SUPP;
			} else {
				data->status = MM_SUCCESS; // crash here
			}

			DllMain(pBaseAddress, data->fdwReason, data->lpvReserved);
		}
	}
} 