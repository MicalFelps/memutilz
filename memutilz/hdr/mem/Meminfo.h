#pragma once

#include "mem/Process.h"

namespace mem {
	class Meminfo {
		Process* m_targetProcess;
		std::vector<MEMORY_BASIC_INFORMATION> pages;
		bool m_bIs32Bit{ false };

		bool resolve_bitness();
	public:
		Meminfo() = delete;
		explicit Meminfo(Process* process) noexcept
			: m_targetProcess{ process }
			, m_bIs32Bit{ resolve_bitness() }{
		}
		Process* get_process() { return m_targetProcess; }
		uintptr_t get_module_base(std::wstring_view name);
		uintptr_t get_program_base() { return get_module_base(m_targetProcess->get_name()); }
		void find_page_info();
		std::vector<MEMORY_BASIC_INFORMATION> get_pages() { return pages; }
		bool is32Bit() { return m_bIs32Bit; }
	};

	bool is_readable_page(const MEMORY_BASIC_INFORMATION& mbi);
}