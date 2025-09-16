#ifndef PROCESS_H
#define PROCESS_H

#include "common.h"
#include "mem/Handle.h"

namespace mem {
	class Process {
		std::wstring m_name;
		DWORD m_pid{ 0 };
		DWORD m_mainThread{ 0 };
		std::vector<DWORD> m_threadIDs{};
		bool m_bIsSuspended{ false };

		bool m_bIs32Bit{ false };

		DWORD findPIDByName(std::wstring_view name);
		bool resolveBitness();
	public:
		explicit Process(std::wstring_view name)
			: m_pid{ findPIDByName(name) }
			, m_name{ name } {
			resolveBitness();
		}

		const std::wstring& getName() const noexcept { return m_name; }
		DWORD getPID() const { return m_pid; }
		void setPID(DWORD pid) { m_pid = pid; }

		void findThreadIDs();
		DWORD findMainThread();

		void suspend();
		bool isSuspended() { return m_bIsSuspended; }
		void resume();

		uintptr_t getModuleBase(std::wstring_view name) const;
		uintptr_t getProgramBase() const { return getModuleBase(getName()); }

		bool is32Bit() const { return m_bIs32Bit; }
	};
}

#endif