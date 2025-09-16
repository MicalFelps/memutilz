#include "mem/common.h"
#include "mem/PageInfo.h"
#include "mem/Exception.h"
#include "mem/constants.h"

namespace mem {
	void PageInfo::findPageInfo() {
		pages.clear();
		SIZE_T start_addr{ USERSPACE_START_ADDR };
		SIZE_T end_addr{};
		m_targetProcess->is32Bit()
			? end_addr = USERSPACE_END_32BIT
			: end_addr = USERSPACE_END_64BIT;

		Handle hProc{ OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, m_targetProcess->getPID()) };
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
			else {
				if (GetLastError() == ERROR_INVALID_PARAMETER)
					break;
				throw mem::Exception("VirtualQueryEx Failed");
			}
		}
	}
	bool isReadablePage(const MEMORY_BASIC_INFORMATION& mbi) {
		if (mbi.Protect & PAGE_GUARD) return false;
		return (mbi.Protect & PAGE_READ_FLAGS);
	}
}