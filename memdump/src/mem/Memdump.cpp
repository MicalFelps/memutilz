#include "pch.h"
#include "mem/Memdump.h"
#include "mem/Exception.h"

namespace mem {
	constexpr size_t operator"" _MB(unsigned long long mb) {
		return mb * 1024 * 1024;
	}

	static constexpr size_t SIZE_16MB = 16_MB;
	static constexpr size_t SIZE_32MB = 32_MB;
	static constexpr size_t SIZE_64MB = 64_MB;
	static constexpr size_t SIZE_100MB = 100_MB;
	static constexpr size_t SIZE_500MB = 500_MB;


	bool Memdump::is_readable_page(const MEMORY_BASIC_INFORMATION& mbi) {
		if (mbi.Protect & PAGE_GUARD) return false;
		return (mbi.Protect & PAGE_READ_FLAGS);
	}
	void Memdump::update_memory_layout() {
		m_oldSnapshotSize = m_newSnapshotSize;
		m_meminfo->get_page_info(); // update memory layout

		m_newSnapshotSize = 0;
		for (const auto& mbi : m_meminfo->pages) {
			if (mbi.State == MEM_COMMIT && is_readable_page(mbi)) {
				m_newSnapshotSize += mbi.RegionSize;
			}
		}
	}
	size_t Memdump::get_optimal_buffer_size() const {
		if (m_newSnapshotSize < SIZE_100MB)
			return SIZE_16MB;
		else if (m_newSnapshotSize < SIZE_500MB)
			return SIZE_32MB;
		else
			return SIZE_64MB;
	}
	void Memdump::distribute_regions() {
		size_t offset = 0;
		size_t bufferIndex = 0;
		for (const auto& mbi : m_meminfo->pages) {
			while (offset < m_snapshotBuffers[0].m_size) {
				if (mbi.State == MEM_COMMIT && is_readable_page(mbi)) {
					m_regions.push_back(MemoryRegion{
							mbi.BaseAddress,
							mbi.RegionSize,
							bufferIndex,
							offset,
							mbi.Protect
						});
					offset += mbi.RegionSize;
				}
			}
			++bufferIndex;
			offset = 0;
		}
	}
	void Memdump::make_snapshot_buffers() {
		size_t optimalBufferSize = get_optimal_buffer_size();

		bool bShouldResize = false;
		if (!m_snapshotBuffers.empty()) {
			size_t currentBufferSize = m_snapshotBuffers[0].m_size;
			bShouldResize = (optimalBufferSize > currentBufferSize * 2)
				|| (currentBufferSize > optimalBufferSize * 2);
		}

		if (m_snapshotBuffers.empty() || bShouldResize) {
			m_snapshotBuffers.clear();
			size_t buffers_needed = (m_newSnapshotSize + optimalBufferSize - 1) / optimalBufferSize;

			for (int i = 0; i < buffers_needed; ++i) {
				m_snapshotBuffers.push_back(BufferChunk{ optimalBufferSize });
			}
		}
		else {
			size_t bufferSize = m_snapshotBuffers[0].m_size;
			size_t currCapacity = bufferSize * m_snapshotBuffers.size();

			if (m_newSnapshotSize > currCapacity) {
				size_t additionalBuffers = ((m_newSnapshotSize - currCapacity) + bufferSize - 1) / bufferSize;
				for (size_t i = 0; i < additionalBuffers; ++i) {
					m_snapshotBuffers.push_back(BufferChunk{ bufferSize });
				}
			}
			distribute_regions();
		}
	}
	void Memdump::compute_thread_count() {
		size_t size_mb = m_newSnapshotSize / (1024 * 1024);
		for (const auto& config : configs) {
			if (size_mb <= config.max_size_mb) {
				m_threadCount = config.thread_count;
				return;
			}
		}
		m_threadCount = 8;
	}

	void Memdump::dump() {
		try {
			init();
		}
		catch (const mem::Exception& e) {
			std::cerr << e.full_msg() << '\n';
		}
		catch (...) {
			std::cerr << "[!] Unknown Error\n";
		}

		m_meminfo->m_targetProcess->suspend();
		if (m_meminfo->m_targetProcess->is_suspended()) {
			if (m_threadCount == 1) {



				/*
				for (const auto& mbi : m_meminfo->pages) {
					SIZE_T nb_bytes_read;
					if (!ReadProcessMemory(m_meminfo->m_targetProcess->get_handle(),
						mbi.BaseAddress,
						(LPVOID)currPos,
						mbi.RegionSize,
						&nb_bytes_read
					))
						throw mem::Exception("ReadProcessMemory Failed");
					if (nb_bytes_read != mbi.RegionSize) { // Partial read
						currPos += nb_bytes_read;
						LPVOID new_addr = LPVOID((uintptr_t)mbi.BaseAddress + nb_bytes_read);
						size_t bytes_remaining = mbi.RegionSize - nb_bytes_read;
						if (!ReadProcessMemory(m_meminfo->m_targetProcess->get_handle(),
							new_addr,
							(LPVOID)currPos,
							bytes_remaining,
							&nb_bytes_read
						))
							throw mem::Exception("ReadProcessMemory Failed");
					m_regions.push_back({
						mbi.BaseAddress,
						nb_bytes_read,
						currPos - (uintptr_t)m_snapshotBuffer,
						mbi.Protect
						});

					currPos += nb_bytes_read;
					}
				}
				*/

			}
			//
			// Async stuff
			// 
		}
	}
}