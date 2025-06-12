#pragma once
#include "mem/Process.h"

namespace mem {
	struct Meminfo {
		Process* m_targetProcess;
		std::vector<MEMORY_BASIC_INFORMATION> pages;

		Meminfo() = delete;
		explicit Meminfo(Process* process) noexcept
			: m_targetProcess{ process } {}

		void get_page_info();
	};
}