#pragma once

#include "pch.h"

namespace mem {
	class Exception : public std::runtime_error {
		DWORD m_err{ 0 };
	public:
		explicit Exception(const char* err_msg)
			: std::runtime_error(err_msg)
			, m_err{ GetLastError() } {
			if (m_err == 0)
				m_err = ERROR_UNIDENTIFIED_ERROR;
		}

		DWORD code() const noexcept { return m_err; }
		std::string full_msg() const {
			std::ostringstream oss;
			oss << "[ERR - " << code() << "]" << "\n |--" << what() << "\n |--"  << describe();
			return oss.str();
		}
		
	private:
		std::string describe() const {
			char* msg_buf = nullptr;
			FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				m_err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&msg_buf,
				0,
				nullptr
			);
			std::string msg = msg_buf ? msg_buf : "Unknown error";
			LocalFree(msg_buf);
			return msg;
		}
	};
}