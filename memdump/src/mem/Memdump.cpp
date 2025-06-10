#include "common.h"
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
			if (mbi.State == MEM_COMMIT && is_readable_page(mbi)) {
				if (offset + mbi.RegionSize > m_snapshotBuffers[0].m_size) {
					++bufferIndex;
					offset = 0;

					if (bufferIndex >= m_snapshotBuffers.size())
						throw mem::Exception("[ERR] Not enough buffers allocated for snapshot");
				}

				m_regions.emplace_back(MemoryRegion{
					mbi.BaseAddress,
					mbi.RegionSize,
					bufferIndex,
					offset,
					mbi.Protect
				});

				offset += mbi.RegionSize;
			}
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
				m_snapshotBuffers.emplace_back(BufferChunk{ optimalBufferSize });
			}
		}
		else {
			size_t bufferSize = m_snapshotBuffers[0].m_size;
			size_t currCapacity = bufferSize * m_snapshotBuffers.size();

			if (m_newSnapshotSize > currCapacity) {
				size_t additionalBuffers = ((m_newSnapshotSize - currCapacity) + bufferSize - 1) / bufferSize;
				for (size_t i = 0; i < additionalBuffers; ++i) {
					m_snapshotBuffers.emplace_back(BufferChunk{ bufferSize });
				}
			}
		}
		distribute_regions();
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
				for (const auto& region : m_regions) {
					SIZE_T total_read = 0;
					uintptr_t src = (uintptr_t)region.m_original_addr;
					LPVOID dst = m_snapshotBuffers[region.m_buffer_chunk_idx].m_address;
					SIZE_T size = region.m_size;

					while (total_read < region.m_size) {
						SIZE_T bytes_read = 0;
						if (!ReadProcessMemory(m_meminfo->m_targetProcess->get_handle(),
							(LPCVOID)src,
							(LPVOID)((uintptr_t)dst + total_read),
							size,
							&bytes_read
						)) {
							DWORD code = GetLastError();
							if (code == ERROR_ACCESS_DENIED) {
								break;
								throw mem::Exception("[ERR] Unknown Error Code from ReadProcessMemory");
							}
						}

						if (bytes_read == 0)
							break;

						total_read += bytes_read;
						src += bytes_read;
						size -= bytes_read;
					}
				}
				//
				// Async stuff
				// 
			}
		}
		m_meminfo->m_targetProcess->resume();
	}
}