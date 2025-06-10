#pragma once

#include "common.h"
#include "mem/Handle.h"

namespace mem {
	class Process {
		std::vector<DWORD> m_pids{};
		std::vector<DWORD> m_threads{};
		DWORD m_pid{ 0 };
		DWORD m_mainThread{ 0 };
		Handle m_handle{ nullptr };
		bool m_bIsWoW64{ false };
		bool m_bIsSuspended{ false };

		std::vector<DWORD> find_pids_by_name(std::wstring_view name);
	public:
		explicit Process(std::wstring_view name) noexcept
			: m_pids{ find_pids_by_name(name) } {
		}

		DWORD get_pid(size_t idx = 0) const;
		std::vector<DWORD> get_all_pids() const { return m_pids; }
		void set_pid(DWORD pid);
		HANDLE get_handle() const { return m_handle.get(); }
		void set_handle(HANDLE handle) { m_handle.reset(handle); }

		void open_process(DWORD dwDesiredAccess, size_t idx = 0) { m_handle.reset(OpenProcess(dwDesiredAccess, FALSE, get_pid(idx))); }
		uintptr_t get_module_base(std::wstring_view name, size_t idx = 0);
		bool isWoW64();

		DWORD find_main_thread();
		void get_thread_ids();

		void suspend();
		bool is_suspended() { return m_bIsSuspended; }
	};
}