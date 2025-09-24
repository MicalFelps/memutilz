#include "mem/common.h"
#include "mem/Memdump.h"
#include "mem/Exception.h"
#include "Memdump.h"
#include "Memscan.h"

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
		m_pageinfo->findPageInfo(); // update memory layout
		m_snapshotSize = 0;

		std::unordered_set<LPCVOID> currPages;

		for (const auto& mbi : m_pageinfo->getPages()) {
			if (mbi.State == MEM_COMMIT && isReadablePage(mbi)) {
				currPages.insert(mbi.BaseAddress);
				m_snapshotSize += mbi.RegionSize;
				auto it = m_regions.find(mbi.BaseAddress);
				if (it != m_regions.end()) {
					it->second.m_size = mbi.RegionSize;
					it->second.m_protection = mbi.Protect;
				}
				else {
					m_regions.emplace(mbi.BaseAddress, MemoryRegion{
						mbi.BaseAddress,
						mbi.RegionSize,
						0,
						0,
						0,
						0,
						mbi.Protect });
				}
			}
		}

		// Remove stale memory regions
		for (auto it = m_regions.begin(); it != m_regions.end();) {
			if (currPages.find(it->first) == currPages.end())
				it = m_regions.erase(it);
			else
				++it;
		}

		updateConsecutiveRegions();
	}
	void Memdump::updateConsecutiveRegions() {
		uintptr_t	expectedAddr	{ 0 };
		auto		groupStart		{ m_regions.end() };
		SIZE_T		groupSize		{ 0 };
		int			groupOffset		{ 0 };

		for (auto it = m_regions.begin(); it != m_regions.end(); ++it) {
			auto& [addr, region] = *it;
			uintptr_t currentAddr = reinterpret_cast<uintptr_t>(addr);

			if (currentAddr != expectedAddr) { // new group
				if (groupStart != m_regions.end()) {
					groupStart->second.m_groupSize = groupSize;
				}

				groupStart = it;
				groupSize = region.m_size;
				groupOffset = 0;
				region.m_groupOffset = 0;

			} else { // same group
				groupSize += region.m_size;
				groupOffset++;
				region.m_groupOffset = groupOffset;
			}

			expectedAddr = reinterpret_cast<uintptr_t>(region.m_original_addr) + region.m_size;
		}
		if (groupStart != m_regions.end()) {
			groupStart->second.m_groupSize = groupSize;
		}

		m_sortedByGroupSize.clear();

		for (auto& [addr, region] : m_regions) {
			if (region.m_groupOffset == 0) {
				m_sortedByGroupSize.emplace_back(addr, &region);
			}
		}

		std::sort(m_sortedByGroupSize.begin(), m_sortedByGroupSize.end(),
			[](const auto& a, const auto& b) {
				return a.second->m_groupSize > b.second->m_groupSize;
			});
	}
	SIZE_T Memdump::getOptimalBufferSize() {
		// we pre-allocate any buffers that exceed our normal size

		SIZE_T normalSize{ 0 };
		if (m_snapshotSize < SIZE_100MB)
			normalSize = SIZE_16MB;
		else if (m_snapshotSize < SIZE_500MB)
			normalSize = SIZE_32MB;
		else
			normalSize = SIZE_64MB;

		for (size_t i = 0; i < m_sortedByGroupSize.size(); ++i) {
			if (m_sortedByGroupSize[i].second->m_groupSize > normalSize) {
				m_largeRegionsCount++;
			} else break;
		}

		if (m_largeRegionsCount > 3) { // this is an arbitrary value I chose
			return normalSize * 2;
		}

		return normalSize;
	}
	void Memdump::distributeRegions() {
		m_remainingBuffers.clear();

		m_remainingBuffers.reserve(m_snapshotBuffers.size());
		for (const auto& buffer : m_snapshotBuffers) {
			m_remainingBuffers.push_back(buffer.m_size);
		}

		for (const auto& [addr, startingRegion] : m_sortedByGroupSize) {
			SIZE_T groupSize = startingRegion->m_groupSize;

			SIZE_T bestBuffer{ 0 };
			SIZE_T bestRemaining{ 0 };

			for (size_t i = 0; i < m_remainingBuffers.size(); ++i) {
				if (startingRegion->m_groupSize <= 0x3000) {
					if (m_remainingBuffers[i] < 0x3000)
						continue;
				}

				if (m_remainingBuffers[i] >= groupSize &&
					m_remainingBuffers[i] > bestRemaining) {
					bestBuffer = i;
					bestRemaining = m_remainingBuffers[i];
				}
			}

			if (bestRemaining == 0) {
				throw mem::Exception("Not enough buffers allocated for snapshot");
			}

			startingRegion->m_buffer_idx = bestBuffer;
			startingRegion->m_buffer_offset = m_snapshotBuffers[bestBuffer].m_size - m_remainingBuffers[bestBuffer];
			m_remainingBuffers[bestBuffer] -= groupSize;
		}
	}
	void Memdump::makeSnapshotBuffers() {
		int prevLargeRegionsCount = m_largeRegionsCount;
		m_largeRegionsCount = 0;
		SIZE_T optimalBufferSize = getOptimalBufferSize();
		bool bShouldResize = false;

		if (!m_snapshotBuffers.empty()) {
			SIZE_T currentBufferSize = m_snapshotBuffers[4].m_size;
			bShouldResize = (optimalBufferSize > currentBufferSize * 2) ||
							(currentBufferSize > optimalBufferSize * 2) ||
							(m_largeRegionsCount > prevLargeRegionsCount);
		}

		if (m_snapshotBuffers.empty() || bShouldResize) {
			m_snapshotBuffers.clear(); // if bShouldResize
			
			SIZE_T buffersNeeded{ ((m_snapshotSize + optimalBufferSize - 1) / optimalBufferSize) + 1 };
			if (m_largeRegionsCount > 0) {
				for (size_t i = 0; i < m_largeRegionsCount; ++i) {
					m_snapshotBuffers.emplace_back(BufferChunk{ m_sortedByGroupSize[i].second->m_groupSize });
				}
				if (buffersNeeded > m_largeRegionsCount) buffersNeeded -= m_largeRegionsCount;
				else buffersNeeded = 1;
			}

			for (SIZE_T i = 0; i < buffersNeeded; ++i) {
				m_snapshotBuffers.emplace_back(BufferChunk{ optimalBufferSize });
			}
		} else {
			SIZE_T currCapacity{ 0 };
			for (size_t i = 0; i < m_snapshotBuffers.size(); ++i) {
				currCapacity += m_snapshotBuffers[i].m_size;
			}

			if (m_snapshotSize > currCapacity) {
				SIZE_T additionalBuffers = ((m_snapshotSize - currCapacity) + optimalBufferSize - 1) / optimalBufferSize;
				for (SIZE_T i = 0; i < additionalBuffers; ++i) {
					m_snapshotBuffers.emplace_back(BufferChunk{ optimalBufferSize });
				}
			}
		}

		distributeRegions();
	}
	void Memdump::computeThreadCount() {
		SIZE_T size_mb = m_snapshotSize / (1024 * 1024);
		for (const auto& config : configs) {
			if (size_mb <= config.max_size_mb) {
				m_threadCount = config.thread_count;
				return;
			}
		}
		m_threadCount = 8;
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
			}
			else {
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
		return nullptr;
	}
	auto Memdump::getFirstRegion(std::map<LPCVOID, MemoryRegion>::iterator it) const {
		if (it->second.m_groupOffset == 0)
			return it;

		while (it != m_regions.begin()) {
			auto prev = std::prev(it);
			if (prev->second.m_groupOffset == 0) {
				it = prev;
				break;
			}
		}

		return it;
	}
	auto Memdump::getNextFirstRegion(std::map<LPCVOID, MemoryRegion>::iterator it) const {
		while (it != m_regions.end() && it->second.m_groupOffset > 0) {
			++it;
		}

		if(it != m_regions.end())
			++it;
		return it;
	}

	std::vector<RegionView> Memdump::divide_regions_by_size() {
		std::vector<RegionView> views{};
		views.resize(m_threadCount);

		for (size_t i = 0; i < m_sortedByGroupSize.size(); ++i) {
			size_t threadIndex = i % m_threadCount;
			views[threadIndex].addRegion(m_sortedByGroupSize[i]);
		}

		return views;
	}
	MemoryView Memdump::readBytesAt(LPCVOID address, SIZE_T amount) {
		const MemoryRegion* currentRegion = getRegion(address);
		if (!currentRegion)
			return MemoryView();

		auto startingGroupRegion = getFirstRegion(m_regions.find(currentRegion->m_original_addr));
		uintptr_t addr = reinterpret_cast<uintptr_t>(address);
		uintptr_t regionStart = reinterpret_cast<uintptr_t>(startingGroupRegion->second.m_original_addr);
		uintptr_t regionEnd = regionStart + startingGroupRegion->second.m_groupSize;

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
			const BufferChunk& c = m_snapshotBuffers[startingGroupRegion->second.m_buffer_idx];
			return MemoryView{ (reinterpret_cast<BYTE*>(c.m_address) + startingGroupRegion->second.m_buffer_offset + offset), returnedAmount };
		}
	}

	SIZE_T Memdump::dump() {
		init();
		Handle hRead{ OpenProcess(PROCESS_VM_READ, FALSE, m_targetProcess->getPID()) };
		SIZE_T totalBytesRead{0};

		m_targetProcess->suspend();
		if (m_targetProcess->isSuspended()) {
			if (m_threadCount == 1) {
				auto regions = RegionView{ m_sortedByGroupSize.begin(), m_sortedByGroupSize.end() };
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
			LPVOID dst = m_snapshotBuffers[region->m_buffer_idx].m_address;
			SIZE_T size = region->m_groupSize;

			while (region_read < region->m_groupSize) {
				SIZE_T bytes_read = 0;
				if (!ReadProcessMemory(handle,
					(LPCVOID)src,
					(LPVOID)((uintptr_t)dst + region->m_buffer_offset + region_read),
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