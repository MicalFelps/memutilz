#include "../hdr/coms/coms.h"
#include "../hdr/mem/injection/injection.h"

namespace coms {
	using namespace mem;

	void InitializePipeComms(Process& process) {
		Handle hPipe{ CreateNamedPipe(
			L"\\\\.\\pipe\\coms",
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_BYTE | PIPE_WAIT,
			1,
			1024,
			1024,
			0,
			nullptr
		)};

		if (hPipe.get() == INVALID_HANDLE_VALUE) {
			std::cerr << "Failed to create pipe...\n";
			return;
		}

		injection::ManualMap(process, "agent.dll");
		ConnectNamedPipe(hPipe.get(), nullptr);

		DWORD pid = GetCurrentProcessId();
		DWORD bytesWritten;
		WriteFile(hPipe.get(), &pid, sizeof(DWORD), &bytesWritten, nullptr);
	}
}