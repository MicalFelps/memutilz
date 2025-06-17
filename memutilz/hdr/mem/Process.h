#pragma once

#include "common.h"
#include "mem/Handle.h"

namespace mem {
	class Process {
		std::wstring m_name;
		DWORD m_pid{ 0 };
		DWORD m_mainThread{ 0 };
		std::vector<DWORD> m_threadIDs{};
		bool m_bIsSuspended{ false };

		DWORD find_pid_by_name(std::wstring_view name);
	public:
		explicit Process(std::wstring_view name)
			: m_pid{ find_pid_by_name(name) }
			, m_name{ name }
		{}

		const std::wstring& get_name() const noexcept { return m_name; }
		DWORD get_pid() const { return m_pid; }
		void set_pid(DWORD pid) { m_pid = pid; }

		void find_thread_ids();
		DWORD find_main_thread();

		void suspend();
		bool is_suspended() { return m_bIsSuspended; }
		void resume();
	};
}