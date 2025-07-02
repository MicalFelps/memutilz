#pragma once
#include <capstone/capstone.h>
#include <optional>
#include "mem/memdump.h"


namespace mem {
	class InsnChunk {
		cs_insn* m_insn{ nullptr };
		int m_count{ 0 };

	public:
		InsnChunk() = default;
		InsnChunk(cs_insn* insn, int count) : m_insn{ insn }, m_count{ count } {}

		// no copying allowed
		InsnChunk(const InsnChunk&) = delete;
		InsnChunk& operator=(const InsnChunk&) = delete;

		// only move
		InsnChunk(InsnChunk&& other) noexcept 
			: m_insn{ other.m_insn }
			, m_count{ other.m_count } {
			other.m_insn = nullptr;
			other.m_count = 0;
		}
		InsnChunk& operator=(InsnChunk&& other) noexcept {
			if (this == &other)
				return *this; 

			cleanup();
			m_insn = other.m_insn;
			m_count = other.m_count;
			other.m_insn = nullptr;
			other.m_count = 0;
			return *this;
		}

		const cs_insn* data() const { return m_insn; }
		int size() const { return m_count; }
		bool empty() const { return m_count == 0; }
		const cs_insn& operator[](size_t index) const { return m_insn[index]; }
		const cs_insn* begin() const { return m_insn; }
		const cs_insn* end() const { return m_insn + m_count; }
		bool isValid() const { return m_insn != nullptr && m_count > 0; }

		InsnChunk fromRange(uintptr_t address, size_t maxBytes);

		~InsnChunk() { cleanup(); }

	private:
		void cleanup() {
			if (m_insn) {
				cs_free(m_insn, m_count);
				m_insn = nullptr;
				m_count = 0;
			}
		}
	};

	/*
	Disassembler takes care of caching the disassembly for executable pages of a process.
	It works on an 'if-needed' basis, only calculating the disassembly if it's actually needed.

	The constructor will figure out what memory pages are executable and populate m_pageCache
	*/
	class Disassembler {
		std::shared_ptr<mem::Memdump> m_memdump{ nullptr };

		csh m_csh{ 0 };
		cs_arch m_arch{ CS_ARCH_X86 };
		cs_mode m_mode{ CS_MODE_64 };

		/*
		CachedPage holds the disassembly and memory hash for a
		memory page in the snapshot. The disassembly holds all
		instructions whose start addresses fall within the page
		boundaries.
		*/
		struct CachedRegion {
			uintptr_t m_hash{ 0 };
			uintptr_t m_startAddr{ 0 };
			size_t m_size{ 0 };
			std::optional<InsnChunk> m_disassembly{};
		};

		struct CachedPage {
			uintptr_t m_hash{ 0 };
			const MemoryRegion* m_pRegion{};
			std::optional<InsnChunk> m_disassembly{};
		};

		std::unordered_map<uintptr_t, CachedPage> m_pageCache;
		std::unordered_map<uintptr_t, CachedRegion> m_regionCache;

	public:
		explicit Disassembler(std::shared_ptr<mem::Memdump> memdump);
		~Disassembler() { cs_close(&m_csh); }

		// if address is misaligned, we'll disassemble at a slightly lower address
		InsnChunk disassemble(uintptr_t address, size_t maxBytes);
		InsnChunk disassembleRaw(uintptr_t address, size_t maxBytes);
		uintptr_t alignToInstrStart(uintptr_t address);

		void setSyntax(cs_opt_value syntax) { cs_option(m_csh, CS_OPT_SYNTAX, syntax); }
		csh getHandle() const { return m_csh; }

		bool isJumpInstruction(const cs_insn* insn) const;
		bool isCallInstruction(const cs_insn* insn) const;
		uintptr_t getJumpTarget(const cs_insn* insn) const;

		const MemoryRegion* getExecutableRegion(uintptr_t address) const;
	private:
		void findExecutablePages();
		void hashMemory(CachedPage& pageCache);
		void hashMemory(CachedRegion& regionCache);

		// Because I don't want to write the same function for both caches
		template<typename CacheMap>
		auto getCacheFrom(CacheMap& cache, uintptr_t address) -> std::optional<std::reference_wrapper<typename CacheMap::mapped_type>>;
	};
}