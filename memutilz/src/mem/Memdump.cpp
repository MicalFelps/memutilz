#include "mem/common.h"
#include "mem/Memdump.h"
#include "mem/Exception.h"
#include "Memdump.h"

namespace mem {
	constexpr SIZE_T operator"" _MB(unsigned long long mb) {
		return mb * 1024 * 1024;
	}

	static constexpr SIZE_T SIZE_16MB = 16_MB;
	static constexpr SIZE_T SIZE_32MB = 32_MB;
	static constexpr SIZE_T SIZE_64MB = 64_MB;
	static constexpr SIZE_T SIZE_100MB = 100_MB;
	static constexpr SIZE_T SIZE_500MB = 500_MB;

	void Memdump::updateMemoryLayout() {
		m_meminfo->findPageInfo(); // update memory layout

		m_SnapshotSize = 0;
		for (const auto& mbi : m_meminfo->getPages()) {
			if (mbi.State == MEM_COMMIT && isReadablePage(mbi)) {
				m_SnapshotSize += mbi.RegionSize;
			}
		}
	}
	SIZE_T Memdump::getOptimalBufferSize() const {
		if (m_SnapshotSize < SIZE_100MB)
			return SIZE_16MB;
		else if (m_SnapshotSize < SIZE_500MB)
			return SIZE_32MB;
		else
			return SIZE_64MB;
	}
	void Memdump::distributeRegions() {
		SIZE_T offset = 0;
		SIZE_T bufferIndex = 0;
		for (const auto& mbi : m_meminfo->getPages()) {
			if (mbi.State == MEM_COMMIT && isReadablePage(mbi)) {
				if (offset + mbi.RegionSize > m_snapshotBuffers[0].m_size) {
					++bufferIndex;
					offset = 0;

					if (bufferIndex >= m_snapshotBuffers.size())
						throw mem::Exception("Not enough buffers allocated for snapshot");
				}

				m_regions.try_emplace(mbi.BaseAddress,
					mbi.BaseAddress,
					mbi.RegionSize,
					bufferIndex,
					offset,
					mbi.Protect
				);

				offset += mbi.RegionSize;
			}
		}
	}
	void Memdump::makeSnapshotBuffers() {
		SIZE_T optimalBufferSize = getOptimalBufferSize();

		bool bShouldResize = false;
		if (!m_snapshotBuffers.empty()) {
			SIZE_T currentBufferSize = m_snapshotBuffers[0].m_size;
			bShouldResize = (optimalBufferSize > currentBufferSize * 2)
				|| (currentBufferSize > optimalBufferSize * 2);
		}

		if (m_snapshotBuffers.empty() || bShouldResize) {
			m_snapshotBuffers.clear();
			SIZE_T buffers_needed = ((m_SnapshotSize + optimalBufferSize - 1) / optimalBufferSize) + 1;

			for (SIZE_T i = 0; i < buffers_needed; ++i) {
				m_snapshotBuffers.emplace_back(BufferChunk{ optimalBufferSize });
			}
		}
		else {
			SIZE_T bufferSize = m_snapshotBuffers[0].m_size;
			SIZE_T currCapacity = bufferSize * m_snapshotBuffers.size();

			if (m_SnapshotSize > currCapacity) {
				SIZE_T additionalBuffers = ((m_SnapshotSize - currCapacity) + bufferSize - 1) / bufferSize;
				for (SIZE_T i = 0; i < additionalBuffers; ++i) {
					m_snapshotBuffers.emplace_back(BufferChunk{ bufferSize });
				}
			}
		}
		distributeRegions();
	}
	void Memdump::computeThreadCount() {
		SIZE_T size_mb = m_SnapshotSize / (1024 * 1024);
		for (const auto& config : configs) {
			if (size_mb <= config.max_size_mb) {
				m_threadCount = config.thread_count;
				return;
			}
		}
		m_threadCount = 8;
	}
	std::vector<RegionView> Memdump::divide_regions_by_size() {
		std::vector<RegionView> views{};
		SIZE_T bytesPerThread = m_SnapshotSize / m_threadCount;

		auto threadBegin = m_regions.begin();
		auto iter = m_regions.begin();

		for (SIZE_T i = 0; i < m_threadCount; ++i) {
			SIZE_T currentThreadBytes{ 0 };
			threadBegin = iter;

			if (i == m_threadCount - 1) {
				views.emplace_back(threadBegin, m_regions.end());
				break;
			}

			while (iter != m_regions.end()
				&& currentThreadBytes < bytesPerThread) {
				currentThreadBytes += iter->second.m_size;
				++iter;
			}

			if (threadBegin != iter)
				views.emplace_back(threadBegin, iter);
		}

		return views;
	}

	const RegionContext Memdump::getRegionContext(LPCVOID address) const {
		if (m_regions.empty()) return {};

		RegionContext ct{};
		uintptr_t addr = (uintptr_t)address;

		auto next = m_regions.upper_bound(address);

		if (next != m_regions.end()) {
			ct.next = &next->second;

			auto nnext = std::next(next);
			if (nnext != m_regions.end()) {
				const auto& nextRegion = next->second;
				uintptr_t nextRegionEnd = (uintptr_t)nextRegion.m_original_addr + nextRegion.m_size;

				if (nextRegionEnd == (uintptr_t)nnext->second.m_original_addr)
					ct.nnext = &nnext->second;
			}
		}


		if (next != m_regions.begin()) {
			auto curr = std::prev(next);

			const auto& region = curr->second;
			uintptr_t regionStart = (uintptr_t)region.m_original_addr;
			uintptr_t regionEnd = regionStart + region.m_size;

			if (addr >= regionStart && addr < regionEnd) {
				ct.curr = &region;

				auto prev = std::prev(curr);
				if (curr != m_regions.begin()) {
					const auto& prevRegion = prev->second;
					uintptr_t prevRegionEnd = (uintptr_t)prevRegion.m_original_addr + prevRegion.m_size;

					if (prevRegionEnd == (uintptr_t)curr->second.m_original_addr)
						ct.prev = &prev->second;
				}
			} else {
				ct.prev = &region;
			}
		}

		return ct;
	}
	const MemoryRegion* Memdump::getRegion(LPCVOID address) const {
		uintptr_t addr = reinterpret_cast<uintptr_t>(address);
		const auto next = m_regions.upper_bound(address);
		if (next != m_regions.begin()) {
			auto curr = std::prev(next);

			const auto& region = curr->second;
			uintptr_t regionStart = (uintptr_t)region.m_original_addr;
			uintptr_t regionEnd = regionStart + region.m_size;

			if (addr >= regionStart && addr < regionEnd) {
				return &curr->second;
			}
		}
	}
	MemoryView Memdump::readBytesAt(LPCVOID address, SIZE_T amount) {
		const RegionContext ct = getRegionContext(address);
		if(!ct.curr)
			return MemoryView();

		uintptr_t addr = reinterpret_cast<uintptr_t>(address);
		uintptr_t regionStart = reinterpret_cast<uintptr_t>(ct.curr->m_original_addr);
		uintptr_t regionEnd = regionStart + ct.curr->m_size;

		SIZE_T offset = addr - regionStart;

		SIZE_T availableBytes = regionEnd - addr;
		SIZE_T returnedAmount = (((amount) < (availableBytes)) ? (amount) : (availableBytes));

		if (m_bLiveMode) {
			if(hRead.get() == nullptr)
				hRead.reset(OpenProcess(PROCESS_VM_READ, FALSE, m_targetProcess->getPID()));

			m_liveBuffer.resize(returnedAmount);
			SIZE_T bytesRead;

			if (ReadProcessMemory(hRead.get(), address, m_liveBuffer.data(), returnedAmount, &bytesRead))
				return MemoryView{ m_liveBuffer.data(), bytesRead };
			return MemoryView();
		} else {
			const BufferChunk& c = m_snapshotBuffers[ct.curr->m_buffer_chunk_idx];
			return MemoryView{ (reinterpret_cast<BYTE*>(c.m_address) + ct.curr->m_buffer_offset + offset), returnedAmount };
		}
	}

	SIZE_T Memdump::dump() {
		init();
		Handle hRead{ OpenProcess(PROCESS_VM_READ, FALSE, m_targetProcess->getPID()) };
		SIZE_T totalBytesRead{0};

		m_targetProcess->suspend();
		if (m_targetProcess->isSuspended()) {
			if (m_threadCount == 1) {
				auto regions = RegionView{ m_regions.begin(), m_regions.end() };
				totalBytesRead = dump_region_view(hRead.get(), regions);
			} else { // Async scanning
				auto allRegions{ divide_regions_by_size() };
				std::vector<std::future<SIZE_T>> futures;

				for (const auto& threadRegions : allRegions) {
					futures.emplace_back(
						std::async(
							std::launch::async,
							[this, handle = hRead.get()](const RegionView& threadView) {
								return dump_region_view(handle, threadView);
							},
							threadRegions
						));
				}
				for (auto& future : futures) {
					totalBytesRead += future.get();
				}
			}
		}
		m_targetProcess->resume();
		return totalBytesRead;
	}
	SIZE_T Memdump::dump_region_view(HANDLE handle, const RegionView& regionView) {
		SIZE_T total_bytes_read = 0;

		for (const auto& [originalAddress, region] : regionView) {
			SIZE_T region_read = 0;
			uintptr_t src = (uintptr_t)originalAddress;
			LPVOID dst = m_snapshotBuffers[region.m_buffer_chunk_idx].m_address;
			SIZE_T size = region.m_size;

			while (region_read < region.m_size) {
				SIZE_T bytes_read = 0;
				if (!ReadProcessMemory(handle,
					(LPCVOID)src,
					(LPVOID)((uintptr_t)dst + region.m_buffer_offset + region_read),
					size,
					&bytes_read
				)) {
					DWORD code = GetLastError();
					if (code == ERROR_ACCESS_DENIED) {
						break;
					}
				}

				if (bytes_read == 0)
					break;

				total_bytes_read += bytes_read;
				region_read += bytes_read;
				src += bytes_read;
				size -= bytes_read;
			}
		}
		return total_bytes_read;
	}
}