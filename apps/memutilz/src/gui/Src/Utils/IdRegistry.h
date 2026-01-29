#pragma once

#include <optional>
#include <QReadWriteLock>
#include "Types.h"

namespace Utils {
	struct Id;
	struct StringHolder;
	class IdCache;
	class IdRegistry {
	public:
		[[nodiscard]] static Id getOrRegister(const char* s, size_t len = 0);
		[[nodiscard]] static std::string_view stringFromId(Id id);
		[[nodiscard]] static std::optional<Id> idFromString(const char* s, size_t len = 0);

	private:
		IdRegistry() = delete;
		~IdRegistry() = delete;

		static QHash<IdValueType, StringHolder> _stringFromId;
		static IdCache _idFromString;
		static QReadWriteLock _mutex;
	};
} // namespace Utils