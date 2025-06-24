#ifndef PRIVILEGE_H
#define PRIVILEGE_H

#include "Windows.h"

namespace utilz {
	namespace privilege {
		bool isRunningAsAdmin();
		bool tryElevate();
		bool CheckAndRequestAdmin();
	}
}

#endif