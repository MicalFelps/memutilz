#include "../mem/injection/injection.h"
#include "../mem/Exception.h"
#include <filesystem>
#include <fstream>

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

		bool ManualMap(Process& targetProc, std::string_view dllPath, bool supportSEH) {
			auto fileBuffer = readPeFile(dllPath);
			BYTE* pBaseAddress = nullptr;

			auto pOldNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(fileBuffer.data() + reinterpret_cast<IMAGE_DOS_HEADER*>(fileBuffer.data())->e_lfanew);
			auto pOldFileHeader = &pOldNtHeader->FileHeader;
			auto pOldOptHeader = &pOldNtHeader->OptionalHeader;

			// Here I'm assuming we're x64, if not we can check that the DLL is valid for that
			if (targetProc.is32Bit() && !pOldFileHeader->Machine == IMAGE_FILE_MACHINE_I386) {
				std::cerr << "Can't inject an x64 DLL into a WoW process\n";
				return false;
			}

			std::cout << "[!] Loading File\n";

			Handle hProc{ OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProc.getPID()) };
			pBaseAddress = reinterpret_cast<BYTE*>(VirtualAllocEx(hProc.get(), nullptr, pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_WRITECOPY));

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
					std::cout << '[' << (i + 1) << ']' << ' ' << "Mapped: " << pSectionHeader->Name << '\n';
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
			data.fpLoadLibrary = LoadLibraryW;
			data.fpGetProcAddress = GetProcAddress;
#ifdef WIN64
			data.fpRtlAddFunctionTable = (fp_RtlAddFunctionTable)RtlAddFunctionTable;
#else
			supportSEH = false;
#endif
			data.baseAddress = reinterpret_cast<uintptr_t>(pBaseAddress);
			data.supportSEH = supportSEH;

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

			std::cout << "[+] Manual Mapping Data at: 0x" << std::hex << reinterpret_cast<uintptr_t>(mmDataLoc) << '\n';

			// Writing the shellcode
			LPVOID shellcodeLoc = VirtualAllocEx(hProc.get(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (!shellcodeLoc) {
				VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
				VirtualFreeEx(hProc.get(), mmDataLoc, 0, MEM_RELEASE);
				throw mem::Exception("Failed to allocate memory for shellcode");
			}

			if (!WriteProcessMemory(hProc.get(), shellcodeLoc, shellcode, 0x1000, nullptr)) {
				VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
				VirtualFreeEx(hProc.get(), mmDataLoc, 0, MEM_RELEASE);
				VirtualFreeEx(hProc.get(), shellcodeLoc, 0, MEM_RELEASE);
				throw mem::Exception("Failed to map shellcode to target process");
			}

			std::cout << "[+] Shellcode at : 0x" << std::hex << reinterpret_cast<uintptr_t>(shellcodeLoc) << '\n';

			std::cout << "	SETUP DONE	" << '\n';
			std::cout << "[!!] Creating Remote Thread" << '\n';

			Handle hRemote{ CreateRemoteThread(hProc.get(), nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(shellcodeLoc), mmDataLoc, 0, nullptr) };
			if (!hRemote) {
				VirtualFreeEx(hProc.get(), pBaseAddress, 0, MEM_RELEASE);
				VirtualFreeEx(hProc.get(), mmDataLoc, 0, MEM_RELEASE);
				VirtualFreeEx(hProc.get(), shellcodeLoc, 0, MEM_RELEASE);
				throw mem::Exception("Failed to create remote thread in target process");
			}

			std::cout << "[+] Thread Created Successfully!" << '\n';

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
				}
				
				/*
				1. SEH Support failure

				if(checkFlag & MM_NO_SEHSUPPORT){
					std::cout << "[-] SEH Exception Support Failed!\n";
				}
				*/

				Sleep(100);
			}

			// Clear PE Headers
			std::vector<BYTE> nullBuffer(0x1000 * 0x10, 0x0);
			if (!WriteProcessMemory(hProc.get(), pBaseAddress, nullBuffer.data(), 0x1000, nullptr)) {
				throw mem::Exception("Failed to clear PE Headers");
			}

			// Clear Un-needed sections
			pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
			for (size_t i = 0; i < pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
				if (pSectionHeader->Misc.VirtualSize) {
					if(supportSEH ? false : strcmp((char*)pSectionHeader->Name, ".pdata") == 0 ||
						strcmp((char*)pSectionHeader->Name, ".rsrc") == 0 ||
						strcmp((char*)pSectionHeader->Name, ".reloc") == 0) {

						std::cout << '[' << (i + 1) << ']' << ' ' << "Clearing: " << pSectionHeader->Name << '\n';
						if (!WriteProcessMemory(hProc.get(), pBaseAddress, nullBuffer.data(), pSectionHeader->Misc.VirtualSize, nullptr)) {
							throw mem::Exception("Failed to clear section during manual mapping");
						}
					}
				}
			}

			
		}

#define RELOC_FLAG32(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else
#define RELOC_FLAG RELOC_FLAG32
#endif

		#pragma optimize("", off)
		#pragma runtime_checks("", off)
		#pragma check_stack(off)

		void __stdcall shellcode(MM_Data* data) {
			BYTE* pBaseAddress = reinterpret_cast<BYTE*>(data->baseAddress);
			auto pOldNtHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<IMAGE_DOS_HEADER*>(data->baseAddress)->e_lfanew);
			auto pOldOptheader = &pOldNtHeaders->OptionalHeader;

			auto fpLoadLibrary = data->fpLoadLibrary;
			auto fpGetProcAddress = data->fpGetProcAddress;
#ifdef WIN64
			auto fpRtlAddFunctionTable = data->fpRtlAddFunctionTable;
#endif
			auto DllMain = reinterpret_cast<fp_DLL_ENTRY>(pBaseAddress + pOldOptheader->AddressOfEntryPoint);

			// 1. Sanity Check for correct addresses of LoadLibrary and GetProcAddress

			// FIX RELOCATION
			// FIX IAT
			// FIX TLS CALLBACKS


		}
	}
} 