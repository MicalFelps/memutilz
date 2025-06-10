#include "common.h"
#include "mem/Process.h"
#include "mem/Handle.h"
#include "mem/Exception.h"

namespace mem {
	std::vector<DWORD> Process::find_pids_by_name(std::wstring_view name) {
		std::vector<DWORD> pids;
		Handle hSnap{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };
		if (hSnap.get() != INVALID_HANDLE_VALUE) {
			PROCESSENTRY32 procEntry{};
			procEntry.dwSize = sizeof(PROCESSENTRY32);

			if (Process32First(hSnap.get(), &procEntry)) {
				do {
					if (!_wcsicmp(procEntry.szExeFile, name.data()))
						pids.push_back(procEntry.th32ProcessID);
				} while (Process32Next(hSnap.get(), &procEntry));
			}
			if (pids.empty())
				throw mem::Exception("Failed to get the process ID");
			else if (pids.size() == 1)
				m_pid = pids[0];
			return pids;
		}
	}
	uintptr_t Process::get_module_base(std::wstring_view name, size_t idx) {
		uintptr_t ret{};
		Handle hSnap{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_pids[idx]) };
		
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
	}

	bool Process::isWoW64() {
		assert(m_handle.get());
		BOOL bIsWoW64 = FALSE;

		using LPFN_ISWOW64PROCESS = BOOL(WINAPI*)(HANDLE, PBOOL);
		LPFN_ISWOW64PROCESS pFnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");

		if (pFnIsWow64Process)
			pFnIsWow64Process(m_handle.get(), &bIsWoW64);

		SYSTEM_INFO sysinfo;
		GetNativeSystemInfo(&sysinfo);

		bool bIs64bitOS{ sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 };
		return (bIs64bitOS && bIsWoW64);
	}
	DWORD Process::get_pid(size_t idx) const {
		if (idx > m_pids.size())
			return m_pids[0];
		else return m_pids[idx];
	}

	void Process::set_pid(DWORD pid) {
		m_pid = pid;
	}

	DWORD Process::find_main_thread() {
		get_thread_ids();
		FILETIME earliestCreationTime{};
		bool first = true;

		for (const auto& t : m_threads) {
			Handle hThread{OpenThread(THREAD_QUERY_INFORMATION, FALSE, t)};
			if (hThread.get() == INVALID_HANDLE_VALUE)
				throw mem::Exception("Failed to get handle to thread");
			
			FILETIME creationTime, exitTime, kernelTime, userTime;
			if (!GetThreadTimes(hThread.get(), &creationTime, &exitTime, &kernelTime, &userTime))
				throw mem::Exception("GetThreadTimes Failed");

			if (first || CompareFileTime(&creationTime, &earliestCreationTime) < 0) {
				m_mainThread = t;
				earliestCreationTime = creationTime;
				first = false;
			}
		}
		return m_mainThread;
	}
	void Process::get_thread_ids() {
		if (m_pid == 0)
			m_pid = get_pid(0);
		Handle hSnap{ CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, m_pid) };
		if (hSnap.get() != INVALID_HANDLE_VALUE) {
			THREADENTRY32 threadEntry{};
			threadEntry.dwSize = sizeof(THREADENTRY32);
			
			if (Thread32First(hSnap.get(), &threadEntry)) {
				do {
					if (threadEntry.th32OwnerProcessID == m_pid) {
						m_threads.push_back(threadEntry.th32ThreadID);
					}
				} while (Thread32Next(hSnap.get(), &threadEntry));
			}
			if (m_threads.size() == 1)
				m_mainThread = m_threads[0];
		}

	}
	
	void Process::suspend() {
		if(m_mainThread == 0)
			find_main_thread();
		else
			get_thread_ids();

		Handle hMainThread{ OpenThread(THREAD_SUSPEND_RESUME, FALSE, m_mainThread) };
		if (SuspendThread(hMainThread.get()) == -1)
			throw mem::Exception("SuspendThread Failed");
		for (const auto& t : m_threads) {
			if (t == m_mainThread)
				continue;
			Handle hThread{ OpenThread(THREAD_SUSPEND_RESUME, FALSE, t) };
			if (SuspendThread(hThread.get()) == -1)
				throw mem::Exception("SuspendThread Failed");
		}
		m_bIsSuspended = true;
	}
	void Process::resume() {
		Handle hMainThread{ OpenThread(THREAD_SUSPEND_RESUME, FALSE, m_mainThread) };
		if (ResumeThread(hMainThread.get()) == -1)
			throw mem::Exception("ResumeThread Failed");
		for (const auto& t : m_threads) {
			if (t == m_mainThread)
				continue;
			Handle hThread{ OpenThread(THREAD_SUSPEND_RESUME, FALSE, t) };
			if (ResumeThread(hThread.get()) == -1)
				throw mem::Exception("ResumeThread Failed");
		}
		m_bIsSuspended = false;
	}
}