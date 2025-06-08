#include "pch.h"

#include "mem/Process.h"
#include "mem/Meminfo.h"
#include "mem/Memdump.h"
#include "mem/Exception.h"

int wmain(int argc, wchar_t** argv) {
	try {
		mem::Process proc{ L"ac_client.exe" };

		proc.set_handle(OpenProcess(PROCESS_ALL_ACCESS, 0, proc.get_pid()));

		mem::Meminfo meminfo(&proc);
		meminfo.get_page_info();
		
		mem::Memdump memdump(&meminfo);
		memdump.dump();

		Sleep(10000);
	}
	catch (const mem::Exception& e) {
		std::cerr << e.full_msg() << '\n';
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << '\n';
	}
	catch (...) {
		std::cerr << "[!] Unknown Error\n";
	}
}