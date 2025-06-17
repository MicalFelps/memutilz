#pragma once

#include <windows.h>

namespace mem {
	inline constexpr SIZE_T USERSPACE_START_ADDR = 0x0;
	inline constexpr SIZE_T USERSPACE_END_32BIT = 0x7FFFFFFF;
	inline constexpr SIZE_T USERSPACE_END_64BIT = 0x00007FFFFFFFFFFF;

	inline constexpr int PAGE_READ_FLAGS =
		PAGE_READONLY |
		PAGE_READWRITE |
		PAGE_WRITECOPY |
		PAGE_EXECUTE_READ |
		PAGE_EXECUTE_READWRITE |
		PAGE_EXECUTE_WRITECOPY;
}