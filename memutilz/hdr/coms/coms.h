#pragma once

#include "mem/Process.h"
#include "mem/injection/injection.h"

//memutilz
namespace coms {
	using namespace mem;

	void InitializePipeComms(Process& process);
}