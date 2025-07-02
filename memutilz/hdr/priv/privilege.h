#ifndef PRIVILEGE_H
#define PRIVILEGE_H

#include "Windows.h"

namespace priv {
	bool isRunningAsAdmin();
	bool tryElevate();
	bool CheckAndRequestAdmin();
}

#endif