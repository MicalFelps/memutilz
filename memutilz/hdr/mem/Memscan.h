#ifndef MEMSCAN_H
#define MEMSCAN_H

#include "mem/Memdump.h"
#include "mem/PageInfo.h"
#include "mem/constants.h"

#include <array>
#include <optional>
#include <immintrin.h>

namespace mem {
    constexpr BYTE hexFromChar(unsigned char c) {
        return (c >= '0' && c <= '9') ? c - '0' :
            (c >= 'A' && c <= 'F') ? c - 'A' + 10 :
            (c >= 'a' && c <= 'f') ? c - 'a' + 10 :
            0xFF; // invalid
    }

    constexpr std::array<BYTE, 256> initHexTable() {
        std::array<BYTE, 256> table{};
        for (int i = 0; i < 256; ++i) {
            table[i] = hexFromChar(static_cast<unsigned char>(i));
        }
        return table;
    }

    constexpr auto hexTable = initHexTable();

    class Memscan{
    public:
        class Pattern {
            std::vector<BYTE> m_bytes;
            std::vector<BYTE> m_mask;

            bool m_bHasWildcards{ false };

            bool m_simdFriendly{ false };
            size_t m_vectorSize{ 0 }; // 32 if using AVX, 16 if SSE
            size_t m_simdBlocks{ 0 }; // patternLen / m_vectorSize

            uintptr_t m_hash{ 0 };

        public:
            Pattern(const std::vector<BYTE>& bytes, const std::vector<BYTE>& mask);
            Pattern(std::vector<BYTE>&& bytes, std::vector<BYTE>&& mask) noexcept;
            Pattern(const std::string_view pattern, const std::string_view mask); // old pattern + mask combo
            explicit Pattern(const std::string_view inlinePattern); // inline wildcards of type "48 8B ?? 74 ??"

            static Pattern fromString(
                const std::string_view str,
                const std::optional<std::string_view>& mask = std::nullopt);

            Pattern(const Pattern& other) = default;
            Pattern(Pattern&& other) noexcept;
            Pattern& operator=(const Pattern& other) = default;
            Pattern& operator=(Pattern&& other) noexcept;

            const BYTE* getBytes() const noexcept { return m_bytes.data(); }
            const BYTE* getMask() const noexcept { return m_mask.data(); }
            auto getHash() const noexcept { return m_hash; }

            size_t size() const noexcept { return m_bytes.size(); }
            bool empty() const noexcept { return m_bytes.empty(); }
            bool hasWildcards() const noexcept { return m_bHasWildcards; }

            bool isSimdFriendly() const noexcept { return m_simdFriendly; } // is it efficient?
            size_t getSimdIterations() const noexcept { return m_simdBlocks; }

            bool match(const BYTE* data) const noexcept;
            bool matchSimd(const BYTE* data) const noexcept;

        private:
            void parseOldFormat(std::string_view pattern, std::string_view mask);
            void parseInlineFormat(std::string_view inlinePattern);
            void checkSimdCompatibility();
        };

        struct ScanOptions {
            DWORD pageProtectionFlags = PAGE_READ_FLAGS;
            size_t alignment = 1;
            bool useSIMD = true;
        };

        struct ScanResult {
            ScanOptions opt;
            uintptr_t patternHash;
            std::vector<uintptr_t> addresses;
            bool success = false;
        };

    private:
        struct ThreadConfig {
            SIZE_T max_size_mb;
            SIZE_T thread_count;
        };
        static constexpr ThreadConfig configs[] = {
            {50, 1},
            {100, 2},
            {500, 4},
            {UINT64_MAX, 8}
        };


        Memdump* m_memdump; // this should really be std::shared_ptr<Memdump> but hey what could go wrong
        PageInfo* m_pageinfo; // same here...
        RegionView m_filteredRegions;

        size_t m_threadCount{ 1 };
        std::unordered_map<uintptr_t, class Pattern> m_patternCache; // nice if we're scanning multiple times

        void filterRegions(DWORD pageProtectionFlags = PAGE_READ_FLAGS);
        void computeThreadCount();
        std::vector<RegionView> divideRegionsBySize() const;

        std::vector<uintptr_t> scanRegionView(
            const RegionView& regionView,
            const Pattern& pattern,
            const ScanOptions& opt) const;

    public:
        Memscan() = delete;
        explicit Memscan(Memdump* memdump)
            : m_memdump{ memdump }
            , m_pageinfo(m_memdump ? m_memdump->getPageinfo() : nullptr) {
        }

        ~Memscan() = default;

        Memscan(const Memscan&) = delete;
        Memscan(Memscan&& other) noexcept;
        Memscan& operator=(const Memscan&) = delete;
        Memscan& operator=(Memscan&& other) noexcept;

        ScanResult ScanPattern(
            const Pattern& pattern,
            ScanOptions opt);

        template<typename T>
        ScanResult ScanValue(
            T value,
            ScanOptions opt)
        {
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
            static_assert(sizeof(T) <= 32, "Value is too big for efficient scanning");

            std::vector<BYTE> bytes(sizeof(T));
            std::memcpy(bytes.data(), &value, sizeof(T));
            std::vector<BYTE> mask(sizeof(T), 0xFF); // no wildcards

            Pattern pattern(std::move(bytes), std::move(mask));
            return ScanPattern(pattern, opt);
        }

        // AOB
        ScanResult ScanBytes(const BYTE* bytes, size_t length, ScanOptions opt = {});
        ScanResult ScanBytes(const std::vector<BYTE>& bytes, ScanOptions opt = {}) {
            return ScanBytes(bytes.data(), bytes.size(), opt);
        }

        ScanResult ScanString(
            std::string_view str,
            ScanOptions opt,
            bool caseSensitive = true);

        // Wide string scanning
        ScanResult ScanWideString(
            std::wstring_view wstr,
            ScanOptions opt,
            bool caseSensitive = true);
    };
}

#endif // MEMSCAN_H