#pragma once

#include "common.h"
#include <sstream>
#include <stdexcept>

namespace mem {
	class Exception : public std::runtime_error {
		DWORD m_err{ 0 };
	public:
		explicit Exception(const char* err_msg)
			: std::runtime_error(err_msg) {
			m_err = GetLastError();
			if (m_err == 0)
				m_err = ERROR_UNIDENTIFIED_ERROR;
		}
		std::string full_msg() const {
			std::ostringstream oss;
			oss << "[ERR - " << code() << "]" << "\n" << what() << "\n"  << describe();
			return oss.str();
		}
		DWORD code() const noexcept { return m_err; }
		
	private:
		std::string describe() const {
			char* buffer = nullptr;
			FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				m_err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&buffer,
				0,
				nullptr
			);
			std::string msg = buffer ? buffer : "Unknown error";
			if(buffer)
				LocalFree(buffer);
			return msg;
		}
	};
}