#pragma once

namespace mem {
	class Handle {
		HANDLE m_handle{ nullptr };
	public:
		Handle() = delete;
		explicit Handle(HANDLE handle) noexcept
			: m_handle{handle} {}
		explicit Handle(DWORD pid, DWORD dwDesiredAccess = PROCESS_ALL_ACCESS)
			: m_handle{ OpenProcess(dwDesiredAccess, false, pid) } {}

		Handle(Handle&& other) noexcept
			: m_handle{ std::exchange(other.m_handle, nullptr) } {}

		Handle& operator=(Handle&& other) noexcept {
			if (this == &other)
				return *this;

			if (m_handle)
				CloseHandle(m_handle);

			m_handle = std::exchange(other.m_handle, nullptr);

			return *this;
		}

		Handle(const Handle&) = delete;
		Handle& operator=(const Handle&) = delete;
		
		~Handle() {
			if (m_handle)
				CloseHandle(m_handle);
		}

		HANDLE get() const { return m_handle; }
		void reset(HANDLE handle) {
			if (m_handle)
				CloseHandle(m_handle);
			m_handle = handle;
		}
		HANDLE release() {
			HANDLE tmp = m_handle;
			m_handle = nullptr;
			return tmp;
		}
		bool operator!() const { return (m_handle == nullptr); }
	};
}