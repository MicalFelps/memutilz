#include "MemorySource.h"
#include <Zydis/Zydis.h>

std::string basic_zydis_disasm(const uint8_t* bytes, size_t length)
{
    ZydisMachineMode mode = ZYDIS_MACHINE_MODE_LONG_64;
    ZydisStackWidth stack_width = ZYDIS_STACK_WIDTH_64;

    if (length == 0) {
        return "[empty buffer]";
    }

    ZydisDecoder decoder;
    ZyanStatus status = ZydisDecoderInit(&decoder, mode, stack_width);
    if (ZYAN_FAILED(status)) {
        return "[error] Failed to initialize Zydis decoder";
    }

    std::ostringstream ss;
    ss << "Basic disassembly (" << (mode == ZYDIS_MACHINE_MODE_LONG_64 ? "x64" : "x86") << "):\n";
    ss << "----------------------------------------\n";

    size_t offset = 0;
    size_t count = 0;

    while (offset < length && count < 15) {
        ZydisDecodedInstruction instr;

        status = ZydisDecoderDecodeInstruction(
            &decoder,
            nullptr,
            bytes + offset,
            length - offset,
            &instr
        );

        if (ZYAN_FAILED(status)) {
            ss << std::hex << std::setw(4) << std::setfill('0') << offset
                << " | db " << std::hex << std::setw(2) << static_cast<unsigned>(bytes[offset])
                << "  ; invalid byte\n";
            offset++;
            continue;
        }

        const char* mnemonic_str = ZydisMnemonicGetString(instr.mnemonic);
        if (!mnemonic_str) {
            mnemonic_str = "[unknown mnemonic]";
        }

        ss << std::hex << std::setw(4) << std::setfill('0') << offset
            << " | " << mnemonic_str
            << "    (length: " << static_cast<int>(instr.length) << " bytes)   ";

        // Quick hex dump
        ss << "[ ";
        for (size_t i = 0; i < instr.length; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<unsigned>(bytes[offset + i]) << " ";
        }
        ss << "]\n";

        offset += instr.length;
        count++;
    }

    if (offset < length) {
        ss << "... (truncated after " << count << " instructions)\n";
    }

    ss << "----------------------------------------\n";
    return ss.str();
}

