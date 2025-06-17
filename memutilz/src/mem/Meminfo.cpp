#include "mem/common.h"
#include "mem/Meminfo.h"
#include "mem/Exception.h"
#include "mem/constants.h"

namespace mem {
	bool Meminfo::resolve_bitness() {
		Handle hProc{ OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, m_targetProcess->get_pid()) };
		BOOL bIsWoW64 = FALSE;

		if (!IsWow64Process(hProc.get(), &bIsWoW64))
			throw mem::Exception("IsWow64Process failed");

		SYSTEM_INFO sysinfo;
		GetNativeSystemInfo(&sysinfo);

		bool bIs64bitOS{ sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 };
		return (bIs64bitOS && bIsWoW64);
	}
	uintptr_t Meminfo::get_module_base(std::wstring_view name) {
		uintptr_t ret{};
		Handle hSnap{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_targetProcess->get_pid()) };

		if (hSnap.get() != INVALID_HANDLE_VALUE) {
			MODULEENTRY32 moduleEntry{};
			moduleEntry.dwSize = sizeof(MODULEENTRY32);

			if (Module32First(hSnap.get(), &moduleEntry)) {
				do {
					if (!_wcsicmp(moduleEntry.szModule, name.data())) {
						ret = (uintptr_t)moduleEntry.modBaseAddr;
						return ret;
					}
				} while (Module32Next(hSnap.get(), &moduleEntry));
			}
			if (!ret)
				throw mem::Exception("Failed to get module base address");
		}
		throw mem::Exception("Failed to get module snapshot");
	}
	void Meminfo::find_page_info() {
		pages.clear();
		SIZE_T start_addr{ USERSPACE_START_ADDR };
		SIZE_T end_addr{};
		is32Bit()
			? end_addr = USERSPACE_END_32BIT
			: end_addr = USERSPACE_END_64BIT;

		Handle hProc{ OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, m_targetProcess->get_pid()) };
		MEMORY_BASIC_INFORMATION mbi{};
		while (start_addr < end_addr) {
			if (VirtualQueryEx(hProc.get(), (LPCVOID)start_addr, &mbi, sizeof(mbi))) {
				if (mbi.RegionSize == 0) {
					start_addr++;
					continue;
				}
				pages.push_back(mbi);
				start_addr += mbi.RegionSize;
			}
			else
				throw mem::Exception("VirtualQueryEx Failed");
		}
	}

	bool is_readable_page(const MEMORY_BASIC_INFORMATION& mbi) {
		if (mbi.Protect & PAGE_GUARD) return false;
		return (mbi.Protect & PAGE_READ_FLAGS);
	}
}