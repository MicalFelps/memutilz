#pragma once

#include "Windows.h"

namespace utilz {
	namespace privilege {
		bool isRunningAsAdmin();
		bool tryElevate();
		bool CheckAndRequestAdmin();
	}
}