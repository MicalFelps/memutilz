#ifndef MEMDUMP_H
#define MEMDUMP_H

#include "common.h"

#include "mem/Process.h"
#include "mem/Meminfo.h"
#include "mem/Exception.h"
#include <map>
#include <future>

namespace mem {
	struct MemoryRegion {
		LPCVOID m_original_addr;
		SIZE_T m_size;
		SIZE_T m_buffer_chunk_idx;
		SIZE_T m_buffer_offset;
		DWORD m_protection;
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
		std::map<LPCVOID, MemoryRegion>::const_iterator m_begin;
		std::map<LPCVOID, MemoryRegion>::const_iterator m_end;

		RegionView(auto b, auto e) : m_begin{ b }, m_end{ e } {}
		auto begin() const { return m_begin; }
		auto end() const { return m_end; }
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

		std::vector<BufferChunk> m_snapshotBuffers{};
		SIZE_T m_SnapshotSize{ 0 };
		std::map<LPCVOID, MemoryRegion> m_regions{};

		void updateMemoryLayout();
		void makeSnapshotBuffers();
		SIZE_T getOptimalBufferSize() const;
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
		MemoryView readBytesAt(LPCVOID address, SIZE_T amount);
		Meminfo* getMeminfo() const { return m_meminfo; }
		Process* getProcess() const { return m_targetProcess; }
		SIZE_T dump();
		void setLiveMode() { m_bLiveMode = true; }
	};
}

#endif