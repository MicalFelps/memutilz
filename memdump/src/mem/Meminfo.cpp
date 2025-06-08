#include "pch.h"
#include "mem/Meminfo.h"
#include "mem/Exception.h"

namespace mem {
	inline constexpr SIZE_T USERSPACE_START_ADDR = 0x0;
	inline constexpr SIZE_T USERSPACE_END_32BIT = 0x7FFFFFFF;
	inline constexpr SIZE_T USERSPACE_END_64BIT = 0x00007FFFFFFFFFFF;

	void mem::Meminfo::get_page_info() {
		SIZE_T start_addr{ USERSPACE_START_ADDR };
		SIZE_T end_addr{};
		m_targetProcess->isWoW64()
			? end_addr = USERSPACE_END_32BIT
			: end_addr = USERSPACE_END_64BIT;

		MEMORY_BASIC_INFORMATION mbi{};
		while (start_addr < end_addr) {
			if (VirtualQueryEx(m_targetProcess->get_handle(), (LPCVOID)start_addr, &mbi, sizeof(mbi))) {
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
}