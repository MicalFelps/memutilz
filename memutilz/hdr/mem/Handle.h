#ifndef HANDLE_H
#define HANDLE_H

namespace mem {
	// The sole purpose of this class is just to avoid having to call CloseHandle()
	class Handle {
		HANDLE m_handle{ nullptr };
	public:
		Handle() = delete;
		explicit Handle(HANDLE handle) noexcept
			: m_handle{handle} {}

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
	};
}

#endif