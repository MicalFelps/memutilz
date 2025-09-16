#include "mem/common.h"
#include "mem/Process.h"
#include "mem/Handle.h"
#include "mem/Exception.h"

namespace mem {
	DWORD Process::findPIDByName(std::wstring_view name) {
		DWORD pid{0};
		Handle hSnap{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };

		if (hSnap.get() != INVALID_HANDLE_VALUE) {
			PROCESSENTRY32 procEntry{};
			procEntry.dwSize = sizeof(PROCESSENTRY32);

			if (Process32First(hSnap.get(), &procEntry)) {
				do {
					if (!_wcsicmp(procEntry.szExeFile, name.data())) {
						pid = procEntry.th32ProcessID;
						break;
					}
				} while (Process32Next(hSnap.get(), &procEntry));
			}
			if (pid == 0)
				throw mem::Exception("Failed to get the process ID");
			return pid;
		}
		throw mem::Exception("Failed to get process snapshot");
	}

	void Process::findThreadIDs() {
		m_threadIDs.clear();
		Handle hSnap{ CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, m_pid) };
		if (hSnap.get() != INVALID_HANDLE_VALUE) {
			THREADENTRY32 threadEntry{};
			threadEntry.dwSize = sizeof(THREADENTRY32);

			if (Thread32First(hSnap.get(), &threadEntry)) {
				do {
					if (threadEntry.th32OwnerProcessID == m_pid) {
						m_threadIDs.push_back(threadEntry.th32ThreadID);
					}
				} while (Thread32Next(hSnap.get(), &threadEntry));
			}
			if (m_threadIDs.size() == 0) {
				throw mem::Exception("Failed to get thread IDs");
			}
			return;
		}
		throw mem::Exception("Failed to get thread snapshot");
	}
	DWORD Process::findMainThread() {
		findThreadIDs(); // In case a new thread spawned
		FILETIME earliestCreationTime{};
		bool first = true;

		for (const auto& t : m_threadIDs) {
			Handle hThread{ OpenThread(THREAD_QUERY_INFORMATION, FALSE, t) };
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

	void Process::suspend() {
		if (m_mainThread == 0)
			findMainThread();
		else
			findThreadIDs();

		Handle hMainThread{ OpenThread(THREAD_SUSPEND_RESUME, FALSE, m_mainThread) };
		if (SuspendThread(hMainThread.get()) == -1)
			throw mem::Exception("SuspendThread Failed");
		for (const auto& t : m_threadIDs) {
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
		for (const auto& t : m_threadIDs) {
			if (t == m_mainThread)
				continue;
			Handle hThread{ OpenThread(THREAD_SUSPEND_RESUME, FALSE, t) };
			if (ResumeThread(hThread.get()) == -1)
				throw mem::Exception("ResumeThread Failed");
		}
		m_bIsSuspended = false;
	}

	bool Process::resolveBitness() {
		Handle hProc{ OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, getPID()) };
		BOOL bIsWoW64 = FALSE;

		if (!IsWow64Process(hProc.get(), &bIsWoW64))
			throw mem::Exception("IsWow64Process failed");

		SYSTEM_INFO sysinfo;
		GetNativeSystemInfo(&sysinfo);

		bool bIs64bitOS{ sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 };
		return (bIs64bitOS && bIsWoW64);
	}
	uintptr_t Process::getModuleBase(std::wstring_view name) const {
		uintptr_t ret{};
		Handle hSnap{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, getPID()) };

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
		throw mem::Exception("Failed to get module snapshot");
	}
}