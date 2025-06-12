#pragma once

#include "common.h"

#include "mem/Process.h"
#include "mem/Meminfo.h"
#include "mem/Exception.h"

#define PAGE_READ_FLAGS\
	(PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)

namespace mem {
	struct BufferChunk {
		LPVOID m_address;
		size_t m_size;

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

		explicit BufferChunk(size_t size)
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
	struct MemoryRegion {
		LPCVOID m_original_addr;
		SIZE_T m_size;
		size_t m_buffer_chunk_idx;
		size_t m_buffer_offset;
		DWORD m_protection;
	};
	struct ThreadConfig {
		size_t max_size_mb;
		size_t thread_count;
	};
	static constexpr ThreadConfig configs[] = {
	{50, 1}, {500, 4}, {UINT64_MAX, 8}
	};

	class Memdump {
		Meminfo* m_meminfo{ nullptr };
		size_t m_threadCount{ 1 };
		std::vector<BufferChunk> m_snapshotBuffers{};
		size_t m_oldSnapshotSize{ 0 };
		size_t m_newSnapshotSize{ 0 };
		std::vector<MemoryRegion> m_regions{};
		//std::vector<std::future<size_t>> futures{};

		bool is_readable_page(const MEMORY_BASIC_INFORMATION& mbi);
		void update_memory_layout();
		void make_snapshot_buffers();
		size_t get_optimal_buffer_size() const;
		void distribute_regions();
		void compute_thread_count();

		void init() {
			update_memory_layout();
			make_snapshot_buffers();
			compute_thread_count();
		}
	public:
		Memdump() = delete;
		explicit Memdump(Meminfo* meminfo) noexcept
			: m_meminfo{meminfo}
		{}

		void dump();
	};
}