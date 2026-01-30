#ifndef MEMORYSOURCE_H
#define MEMORYSOURCE_H

#include <cstdint>
#include <optional>
#include <string>
#include <sstream>
#include <iomanip>

/**
 * @brief A test class to verify Doxygen setup
 *
 * This class exists only to test that Doxygen is generating output.
 */
class TestDoc
{
public:
	/**
	 * @brief Test function
	 */
	void doSomething();
};

class MemorySource {
	virtual ~MemorySource() = default;
	virtual size_t read(uint64_t address, void* buffer, size_t length) const = 0;
	virtual std::optional<uint64_t> size() const { return std::nullopt; }
	virtual bool is_valid(uint64_t address, size_t length = 1) const {
		return address + length <= size();
	}
};

int give_10() { return 10; }

#ifdef __linux__
#else
__declspec(dllexport) std::string basic_zydis_disasm(const uint8_t* bytes, size_t length);
#endif

#endif
