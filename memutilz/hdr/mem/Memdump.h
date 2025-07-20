#ifndef MEMDUMP_H
#define MEMDUMP_H

#include "common.h"

#include "mem/Process.h"
#include "mem/Meminfo.h"
#include "mem/Exception.h"
#include <map>
#include <future>
#include <unordered_set>

namespace mem {
	struct MemoryRegion {
		LPCVOID m_original_addr{ 0 };
		SIZE_T m_size{ 0 };
		SIZE_T m_groupSize{ 0 };
		int m_groupOffset{ 0 };
		SIZE_T m_buffer_idx{ 0 };
		SIZE_T m_buffer_offset{ 0 };
		DWORD m_protection{ 0 };
	};
	struct RegionContext {
		const MemoryRegion* prev{ nullptr };
		const MemoryRegion* curr{ nullptr };// nullptr if not applicable
		const MemoryRegion* next{ nullptr };
		const MemoryRegion* nnext{ nullptr };
	};
	struct MemoryView {
		const BYTE* data{ nullptr };
		SIZE_T size{ 0 };
	};
	struct RegionView {
		std::vector<std::pair<LPCVOID, MemoryRegion*>> m_regions;

		RegionView(auto b, auto e) {
			for (auto it = b; it != e; ++it) {
				m_regions.emplace_back(it->first, it->second);
			}
		}

		RegionView() = default;

		void addRegion(const std::pair<LPCVOID, MemoryRegion*>& region) {
			m_regions.push_back(region);
		}

		auto begin() const { return m_regions.begin(); }
		auto end() const { return m_regions.end(); }
	};

	class Memdump {
		struct ThreadConfig {
			SIZE_T max_size_mb;
			SIZE_T thread_count;
		};
		static constexpr ThreadConfig configs[] = {
			{50, 1},
			{500, 4},
			{UINT64_MAX, 8}
		};

		struct BufferChunk {
			LPVOID m_address;
			SIZE_T m_size;

			BufferChunk() = delete;
			BufferChunk(const BufferChunk& other) = delete;
			BufferChunk& operator=(const BufferChunk& other) = delete;

			BufferChunk(BufferChunk&& other) noexcept
				: m_address(other.m_address)
				, m_size(other.m_size)
			{
				other.m_address = nullptr;
				other.m_size = 0;
			}
			BufferChunk& operator=(BufferChunk&& other) noexcept {
				if (this == &other)
					return *this;
				if (m_address) {
					VirtualFree(m_address, 0, MEM_RELEASE);
				}
				m_address = other.m_address;
				m_size = other.m_size;
				other.m_address = nullptr;
				other.m_size = 0;
			}

			explicit BufferChunk(SIZE_T size)
				: m_size{ size }
			{
				m_address = VirtualAlloc(nullptr, m_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
				if (!m_address)
					throw mem::Exception("VirtualAlloc Failed");
			}
			~BufferChunk() {
				if (m_address)
					VirtualFree(m_address, 0, MEM_RELEASE);
			}

		};

		Meminfo* m_meminfo{ nullptr };
		Process* m_targetProcess{ nullptr };

		bool m_bLiveMode{ false };
		std::vector<BYTE> m_liveBuffer;
		Handle hRead{ nullptr };

		SIZE_T m_threadCount{ 1 };

		int m_largeRegionsCount{ 0 };
		std::vector<BufferChunk> m_snapshotBuffers{};
		std::vector<SIZE_T> m_remainingBuffers{};
		SIZE_T m_snapshotSize{ 0 };

		std::map<LPCVOID, MemoryRegion> m_regions{};
		std::vector<std::pair<LPCVOID, MemoryRegion*>> m_sortedByGroupSize;

		void updateMemoryLayout();
		void updateConsecutiveRegions();
		void makeSnapshotBuffers();
		SIZE_T getOptimalBufferSize();
		void distributeRegions();
		void computeThreadCount();
		std::vector<RegionView> divide_regions_by_size();

		void init() {
			updateMemoryLayout();
			makeSnapshotBuffers();
			computeThreadCount();
		}

		SIZE_T dump_region_view(HANDLE handle, const RegionView& regionView);
	public:
		Memdump() = delete;
		explicit Memdump(Meminfo* meminfo) noexcept
			: m_meminfo{meminfo}
			, m_targetProcess{ m_meminfo->getProcess() }
		{}

		const RegionContext getRegionContext(LPCVOID address) const;
		const MemoryRegion* getRegion(LPCVOID address) const;
		auto getFirstRegion(std::map<LPCVOID, MemoryRegion>::iterator it) const;
		auto getNextFirstRegion(std::map<LPCVOID, MemoryRegion>::iterator it) const;
		MemoryView readBytesAt(LPCVOID address, SIZE_T amount);
		Meminfo* getMeminfo() const { return m_meminfo; }
		Process* getProcess() const { return m_targetProcess; }
		SIZE_T dump();
		void setLiveMode() { m_bLiveMode = true; }
	};
}

#endif