#ifndef PAGEINFO_H
#define PAGEINFO_H

#include "mem/Process.h"

namespace mem {
	class PageInfo {
		Process* m_targetProcess;
		std::vector<MEMORY_BASIC_INFORMATION> pages;

	public:
		PageInfo() = delete;
		explicit PageInfo(Process* process) noexcept
			: m_targetProcess{ process }{
		}
		Process* getProcess() const { return m_targetProcess; }
		void findPageInfo();
		std::vector<MEMORY_BASIC_INFORMATION> getPages() { return pages; }
	};

	bool isReadablePage(const MEMORY_BASIC_INFORMATION& mbi);
}

#endif