#pragma once

#include <windows.h>
#include <cstdint>

//agent
namespace coms {
	class FunctionDispatcher;

    struct FunctionResponsePacket {
        uint32_t seqID;
        uint32_t errCode;
        uint32_t retType;
        uint8_t retVal[];
    };

	void InitializeDispatcher();
}