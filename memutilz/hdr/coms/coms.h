#pragma once

#include "mem/Process.h"
#include "mem/injection/injection.h"

//memutilz
namespace coms {
	using namespace mem;

	// This function should:
	// 1. Alloc Console
	// 2. Create Pipe
	// 3. Inject agent.dll
	// After which, we send an isOk packet, and agent.dll confirms
	// We're ready to start requesting function calls
	void InitializePipeComms(Process& process);

	// When asking to call a function, we need
	// 1. The function name / address
	// 2. return type, and it's size
	// 3. the arguments, and their size
	// 4. the calling convention
	// 5. the module the function belongs to (if any)

	enum Convention : uint32_t {
		cdecl_,
		stdcall,
		fastcall,
		thiscall
	};

	enum ParamType : size_t {
		PARAM_VOID = 0,
		PARAM_INT8 = 1,
		PARAM_INT16 = 2,
		PARAM_INT32 = 4,
		PARAM_INT64 = 8,
		PARAM_PTR = sizeof(void*),
		PARAM_FLOAT = 4,
		PARAM_DOUBLE = 8,
		PARAM_STRING = 255
	};

	struct Parameter {
		ParamType type;
		size_t size; // for PARAM_STRING type
		BYTE data[];
	};

	struct FunctionCallRequest {
		uint32_t seq;
		uint32_t size;
		uintptr_t funcAddr;
		char modName[64];
		char funcName[128];
		Convention conv;
		ParamType retType;
		uint32_t argc;
		//Parameter argv[8];
	};

	template<typename RetType, typename... Args>
	RetType CallFunction(Convention conv, uint64_t funcAddr, Args... args);
	/*
	switch(conv){
	case cdecl_: return reinterpret_cast<RetType(__cdecl*)(Args...)>(funcAddr)(args...);
	case stdcall: return reinterpret_cast<RetType(__stdcall*)(Args...)>(funcAddr)(args...);
	case thiscall: return reinterpret_cast<RetType>(__thiscall*)(Args...)<funcAddr>(args...);
    case fastcall: return reinterpret_cast<RetType(__fastcall*)(Args...)>(funcAddr)(args...);
	}
	*/

}