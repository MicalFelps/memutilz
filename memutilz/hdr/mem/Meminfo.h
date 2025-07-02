#ifndef MEMINFO_H
#define MEMINFO_H

#include "mem/Process.h"

namespace mem {
	class Meminfo {
		Process* m_targetProcess;
		std::vector<MEMORY_BASIC_INFORMATION> pages;
		bool m_bIs32Bit{ false };

		bool resolveBitness();
	public:
		Meminfo() = delete;
		explicit Meminfo(Process* process) noexcept
			: m_targetProcess{ process }
			, m_bIs32Bit{ resolveBitness() }{
		}
		Process* getProcess() const { return m_targetProcess; }
		uintptr_t getModuleBase(std::wstring_view name) const;
		uintptr_t getProgramBase() const { return getModuleBase(m_targetProcess->getName()); }
		void findPageInfo();
		std::vector<MEMORY_BASIC_INFORMATION> getPages() { return pages; }
		bool is32Bit() const { return m_bIs32Bit; }
	};

	bool isReadablePage(const MEMORY_BASIC_INFORMATION& mbi);
}

#endif