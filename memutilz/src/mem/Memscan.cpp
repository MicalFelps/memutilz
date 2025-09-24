#include "mem/Memscan.h"
#include "mem/Memdump.h"
#include "hash/xxhash.h"

#include <future>

namespace mem{
    // Constructors
    Memscan::Pattern::Pattern(const std::vector<BYTE>& bytes, const std::vector<BYTE>& mask)
        : m_bytes{bytes}
        , m_mask{mask}
    {
        if (bytes.size() != mask.size())
            std::invalid_argument("[WARN] Pattern's length and mask length don't match!");

        m_bHasWildcards = std::any_of(bytes.begin(), bytes.end(),
            [](BYTE b) {return b == 0x00; });

        checkSimdCompatibility();
    }
    Memscan::Pattern::Pattern(std::vector<BYTE>&& bytes, std::vector<BYTE>&& mask) noexcept
        : m_bytes{ std::move(bytes) }
        , m_mask{ std::move(mask) }
    {
        m_bHasWildcards = std::any_of(bytes.begin(), bytes.end(),
            [](BYTE b) {return b == 0x00; });

        checkSimdCompatibility();
    }
    Memscan::Pattern::Pattern(const std::string_view pattern, const std::string_view mask) {
        parseOldFormat(pattern, mask);
        checkSimdCompatibility();
    }
    Memscan::Pattern::Pattern(const std::string_view inlinePattern) {
        parseInlineFormat(inlinePattern);
        checkSimdCompatibility();
    }

    // Move Constructors
    Memscan::Pattern::Pattern(Pattern&& other) noexcept
        : m_bytes(std::move(other.m_bytes))
        , m_mask(std::move(other.m_mask))
        , m_bHasWildcards(other.m_bHasWildcards)
        , m_simdFriendly(other.m_simdFriendly)
        , m_simdBlocks(other.m_simdBlocks) {
    }
    mem::Memscan::Pattern& mem::Memscan::Pattern::operator=(Pattern&& other) noexcept {
        if (this == &other)
            return *this;

        m_bytes = std::move(other.m_bytes);
        m_mask = std::move(other.m_mask);
        m_bHasWildcards = other.m_bHasWildcards;
        m_simdFriendly = other.m_simdFriendly;
        m_simdBlocks = other.m_simdBlocks;
    }

    // String Parsing
    mem::Memscan::Pattern Memscan::Pattern::fromString(const std::string_view str, const std::optional<std::string_view>& mask) {
        if (mask.has_value()) {
            return Pattern(str, mask.value());
        } else { return Pattern(str); }
    }
    void Memscan::Pattern::parseOldFormat(std::string_view pattern, std::string_view mask) {
        std::string filtered;

        // account for '\x'
        for (size_t i = 0; i < pattern.size(); ++i) {
            if (i + 1 < pattern.size() && pattern[i] == '\\' && pattern[i + 1] == 'x') {
                ++i;
                continue;
            }
            if (pattern[i] != ' ' && pattern[i] != ',') {
                filtered += pattern[i];
            }
        }

        if (filtered.size() % 2 != 0) {
            throw std::invalid_argument("[WARN] Pattern has incomplete byte");
        }

        size_t expectedSize = filtered.size() / 2;
        if (mask.size() != expectedSize) {
            throw std::invalid_argument("[WARN] Mask length must match pattern length");
        }

        m_bytes.reserve(expectedSize);
        m_mask.reserve(expectedSize);

        // Convert to bytes
        for (size_t i = 0; i < filtered.size(); i += 2) {
            char high = filtered[i];
            char low = filtered[i + 1];

            BYTE highNibble = hexTable[static_cast<BYTE>(high)];
            BYTE lowNibble = hexTable[static_cast<BYTE>(low)];

            if (highNibble == 0xFF || lowNibble == 0xFF) {
                throw std::invalid_argument("[WARN] Invalid hex character in pattern");
            }

            m_bytes.push_back((highNibble << 4) | lowNibble);
        }

        for (size_t i = 0; i < mask.size(); ++i) {
            char c = mask[i];
            if (c == 'x') {
                m_mask.push_back(0xFF);
            }
            else if (c == '?') {
                m_mask.push_back(0x00);
                m_bHasWildcards = true;
            }
            else {
                throw std::invalid_argument("[WARN] Mask must only contain 'x' and '?' characters.");
            }
        }
    }
    void Memscan::Pattern::parseInlineFormat(std::string_view inlinePattern) {
        std::string filtered;

        // Remove separators and whitespace, convert to uppercase
        for (size_t i = 0; i < inlinePattern.size(); ++i) {
            char c = inlinePattern[i];

            // Skip \x separators
            if ((i + 1) < inlinePattern.size() && c == '\\' && inlinePattern[i + 1] == 'x') {
                i++; // Skip the 'x' part of '\x'
                continue;
            }

            // Skip whitespace and common separators
            switch (c) {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
            case ',':
            case ';':
            case ':':
                continue;
            default: filtered += std::toupper(c);
            }
        }

        if (filtered.size() % 2 != 0) {
            throw std::invalid_argument("[WARN] Pattern has incomplete byte");
        }

        size_t expectedSize = filtered.size() / 2;
        m_bytes.reserve(expectedSize);
        m_mask.reserve(expectedSize);

        for (size_t i = 0; i < filtered.size(); i += 2) {
            char high = filtered[i];
            char low = filtered[i + 1];

            if (high == '?' && low == '?') {
                m_bytes.push_back(0x00);
                m_mask.push_back(0x00);
                m_bHasWildcards = true;
            }
            else if (high != '?' && low != '?') {
                BYTE highNibble = hexTable[static_cast<unsigned char>(high)];
                BYTE lowNibble = hexTable[static_cast<unsigned char>(low)];

                if (highNibble == 0xFF || lowNibble == 0xFF) {
                    throw std::invalid_argument("Invalid hex character in pattern");
                }

                m_bytes.push_back((highNibble << 4) | lowNibble);
                m_mask.push_back(0xFF);
            }
            else {
                throw std::invalid_argument("Mixed wilcards/hex nibbles aren't supported");
            }
        }
    }

    // Matching and SIMD
    void Memscan::Pattern::checkSimdCompatibility() {
    /*
    Figure out the most best-fit (SSE or AVX) for pattern Length
    Check good fragmentation (no fragmentation) for good performance gains
    Eventually handle variable-length wildcards '*', meaning no SIMD support
    */

        m_hash = XXH3_64bits(m_bytes.data(), m_bytes.size());

#if defined(_MSC_VER)
    // with the help of intrin.h :3
    int cpuInfo[4]{ 0 };
    __cpuid(cpuInfo, 1);
    bool bSSE2 = (cpuInfo[3] & (1 << 26)) != 0;
    __cpuidex(cpuInfo, 7, 0);
    bool bAVX2 = (cpuInfo[1] & (1 << 5)) != 0;
#else
    // For GCC/Clang we use __builtin_cpu_supports
    #if defined(__GNUC__) || defined(__clang__)
    bool bSSE2 = __builtin_cpu_supports("sse2");
    bool bAVX2 = __builtin_cpu_supports("avx2");
    #else
    bool bSSE2 = false; // Fallback: unknown
    bool bAVX2 = false;
    #endif
#endif
    
    if (!bSSE2) {
        m_simdFriendly = false;
        m_simdBlocks = 0;
        return;
    }

    size_t patternLen = m_bytes.size();
    if (patternLen < 16) {
        m_simdFriendly = false;
        m_simdBlocks = 0;
        return;
    }

    size_t wildcardCount = 0;
    for (size_t i = 0; i < patternLen; ++i) {
        if (m_mask[i] == 0x00)
            ++wildcardCount;
    }

    size_t paddingSSE = (16 - (patternLen % 16)) % 16;
    size_t paddingAVX = (32 - (patternLen % 32)) % 32;

    size_t totalSSE = patternLen + paddingSSE;
    double wildcardRatioSSE = static_cast<double>(wildcardCount + paddingSSE) / totalSSE;

    size_t totalAVX = patternLen + paddingAVX;
    double wildcardRatioAVX = static_cast<double>(wildcardCount + paddingAVX) / totalAVX;

    bool sseViable = bSSE2 && wildcardRatioSSE <= 0.40;
    bool avxViable = bAVX2 && wildcardRatioAVX <= 0.40;

    if (!sseViable && !avxViable) {
        m_simdFriendly = false;
        m_simdBlocks = 0;
        return;
    }

    // Choose the better option (prefer the one with the lower ratio)
    if (avxViable && (!sseViable || wildcardRatioAVX < wildcardRatioSSE)) {
        m_vectorSize = 32;
        m_simdBlocks = totalAVX / 32;
    } else {
        m_vectorSize = 16;
        m_simdBlocks = totalSSE / 16;
    }

    // Check for contiguous fixed run, make sure that at least one is >= m_vectorSize/2
    size_t maxFixedRun = 0;
    size_t currRun = 0;
    for (BYTE& b : m_mask) {
        if (b == 0xFF) {
            ++currRun;
            maxFixedRun = (((maxFixedRun) > (currRun)) ? (maxFixedRun) : (currRun));
        } else { currRun = 0; }
    }
    if (maxFixedRun < (m_vectorSize / 2)) {
        m_simdFriendly = false;
        m_simdBlocks = 0;
        return; // too much fragmentation for efficient vectorization
    }

    m_simdFriendly = true;

    if (m_vectorSize == 16) {
        for (size_t i = 0; i < paddingSSE; ++i) {
            m_bytes.push_back(0x00);
            m_mask.push_back(0x00);
        }
    }
    else if (m_vectorSize == 32) {
        for (size_t i = 0; i < paddingAVX; ++i) {
            m_bytes.push_back(0x00);
            m_mask.push_back(0x00);
        }
    }
}
    bool Memscan::Pattern::match(const BYTE * data) const noexcept {
        if (!data) {
            return false;
        }

        for (size_t i = 0; i < m_bytes.size(); ++i) {
            if (m_mask[i] != 0x00 && data[i] != m_bytes[i]) {
                return false;
            }
        }
        return true;
    }
    bool Memscan::Pattern::matchSimd(const BYTE* data) const noexcept {
        if (!data) {
            return false;
        }

        if (!m_simdFriendly) {
            return match(data);
        }

        // Process pattern in vector-sized blocks (16 for SSE, 32 for AVX2)
        if (m_vectorSize == 32) {
            for (size_t block = 0; block < m_simdBlocks; ++block) {
                __m256i vData       = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data + (block * 32)));
                __m256i vPattern    = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(m_bytes.data() + (block * 32)));
                __m256i vMask       = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(m_mask.data() + (block * 32)));

                __m256i xorResult = _mm256_xor_si256(vData, vPattern);
                
                // Apply mask and zero out wildcard positions
                __m256i masked = _mm256_xor_si256(xorResult, vMask);

                if (!_mm256_testz_si256(masked, masked)) {
                    return false; // non-zero bytes means we have a mismatch in cared bytes
                }
            }
        } else if (m_vectorSize == 16) {
            for (size_t block = 0; block < m_simdBlocks; ++block) {
                __m128i vData       = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + (block * 16)));
                __m128i vPattern    = _mm_loadu_si128(reinterpret_cast<const __m128i*>(m_bytes.data() + (block * 16)));
                __m128i vMask       = _mm_loadu_si128(reinterpret_cast<const __m128i*>(m_mask.data() + (block * 16)));

                __m128i xorResult = _mm_xor_si128(vData, vPattern);

                // Apply mask and zero out wildcard positions
                __m128i masked = _mm_xor_si128(xorResult, vMask);

                if (!_mm_testz_si128(masked, masked)) {
                    return false; // non-zero bytes means we have a mismatch in cared bytes
                }
            }
        }
        return true;
    }

    //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //-=-=-=-=-=-=-=-=-=-=-=-MemScan-=-=-=-=-=-=-=-=-=-=-=-
    //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Memscan::Memscan(Memscan&& other) noexcept
        : m_memdump{ other.m_memdump }
        , m_pageinfo{ other.m_pageinfo }
        , m_filteredRegions{ std::move(other.m_filteredRegions) }
        , m_threadCount{ other.m_threadCount }
        , m_patternCache{ std::move(other.m_patternCache) }
    {
        other.m_memdump = nullptr;
        other.m_pageinfo = nullptr;
    }
    Memscan& Memscan::operator=(Memscan&& other) noexcept {
        if (this == &other)
            return *this;

        m_memdump = other.m_memdump;
        m_pageinfo = other.m_pageinfo;
        m_filteredRegions = std::move(other.m_filteredRegions);
        m_threadCount = other.m_threadCount;
        m_patternCache = std::move(other.m_patternCache);

        other.m_memdump = nullptr;
        other.m_pageinfo = nullptr;
    }

    void Memscan::filterRegions(DWORD pageProtectionFlags) {
        if (!m_pageinfo)
            return;

        m_filteredRegions.clear();
        const auto& allRegions = m_memdump->getSortedRegions();

        for (const auto& region : allRegions) {
            if (region.second->m_protection & pageProtectionFlags) {
                m_filteredRegions.addRegion(std::make_pair(region.first, region.second));
            }
        }
    }
    void Memscan::computeThreadCount() {
        size_t totalSize{ 0 };
        for (const auto& region : m_filteredRegions) {
            totalSize += region.second->m_size;
        }
        size_t totalSizeMB{ totalSize / (1024 * 1024) };

        for (const auto& config : configs) {
            if (totalSizeMB <= config.max_size_mb) {
                m_threadCount = config.thread_count;
                return;
            }
        }
        m_threadCount = 8;
    }
    std::vector<RegionView> Memscan::divideRegionsBySize() const {
        std::vector<RegionView> views{};
        views.resize(m_threadCount);

        for (size_t i = 0; i < m_filteredRegions.m_regions.size(); ++i) {
            size_t threadIndex = i % m_threadCount;
            views[threadIndex].addRegion(m_filteredRegions.m_regions[i]);
        }

        return views;
    }

    Memscan::ScanResult Memscan::ScanPattern(const Pattern& pattern, ScanOptions opt) {
        Memscan::ScanResult result{};
        result.opt = opt;
        result.patternHash = pattern.getHash();
        result.success = false;

        if (!m_memdump || !m_pageinfo) {
            return result;
        }

        filterRegions(opt.pageProtectionFlags);
        computeThreadCount();

        if (m_threadCount == 1) {
            const auto& regionView = m_filteredRegions;
            result.addresses = scanRegionView(regionView, pattern, opt);
        } else {
            const auto& regionViews = divideRegionsBySize();
            std::vector<std::future<std::vector<uintptr_t>>> futures;

            for (const auto& threadView : regionViews) {
                futures.emplace_back(
                    std::async(
                        std::launch::async,
                        [this, &threadView, &pattern, &opt]() {
                            return scanRegionView(threadView, pattern, opt);
                        }
                    ));
            }
            for (auto& future : futures) {
                auto vector = future.get();
                result.addresses.insert(
                    result.addresses.end(),
                    std::make_move_iterator(vector.begin()),
                    std::make_move_iterator(vector.end()));
            }
        }
        if (result.addresses.size() != 0) { result.success = true; }
        std::sort(result.addresses.begin(), result.addresses.end());

        return result;
    }

    std::vector<uintptr_t> Memscan::scanRegionView(
        const RegionView& regionView,
        const Pattern& pattern,
        const ScanOptions& opt) const {

        std::vector<uintptr_t> results;
        const auto& snapshotBuffers = this->m_memdump->getSnapshotBuffers();

        for (const auto&[baseAddress, region] : regionView) {
            const BYTE* data = reinterpret_cast<BYTE*>(snapshotBuffers[region->m_buffer_idx].m_address);
            size_t dataSize = region->m_size;
            uintptr_t realAddress = reinterpret_cast<uintptr_t>(baseAddress);

            size_t searchEnd = dataSize - pattern.size() + 1;

            for (size_t i = 0; i < searchEnd; i += opt.alignment) {
                bool matches = false;

                if (opt.useSIMD && pattern.isSimdFriendly()) {
                    matches = pattern.matchSimd(data + i);
                }
                else {
                    matches = pattern.match(data + i);
                }

                if (matches) {
                    results.push_back(realAddress + i);
                }
            }
        }
        return results;
    }

    Memscan::ScanResult Memscan::ScanBytes(const BYTE* bytes, size_t length, ScanOptions opt) {
        return ScanResult();
    }

    Memscan::ScanResult Memscan::ScanString(std::string_view str, ScanOptions opt, bool caseSensitive)
    {
        return ScanResult();
    }

    Memscan::ScanResult Memscan::ScanWideString(std::wstring_view wstr, ScanOptions opt, bool caseSensitive)
    {
        return ScanResult();
    }


}