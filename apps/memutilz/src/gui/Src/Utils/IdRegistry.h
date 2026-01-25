#pragma once

#include <optional>
#include <string_view>
#include <QReadWriteLock>

#include "Types.h"

namespace Utils {

class StringHolder;
class IdCache;

class IdRegistry {
public:
	[[nodiscard]] static Id getOrRegister(const char* s, size_t len = 0);
	[[nodiscard]] static std::string_view stringFromId(Id id);
	[[nodiscard]] static std::optional<Id> idFromString(const char* s, size_t len = 0);

private:
	IdRegistry() = delete;
	~IdRegistry() = delete;

	static QHash<Id::value_type, StringHolder> _stringFromId;
	static IdCache _idFromString;
	static QReadWriteLock _mutex;
};

struct Id {
	using value_type = u64;

	Id() = default;
	Id(const Id&) = default;
	Id& operator=(const Id&) = default;

	template <size_t N>
	Id(const char(&s)[N]) : _id{ IdRegistry::getOrRegister(s, N - 1) } {}

	explicit Id(value_type raw) : _id{ raw } {}
	explicit Id(const char* s) : _id{ IdRegistry::getOrRegister(s) } {}
	explicit Id(std::string s) : _id{ IdRegistry::getOrRegister(s.data(), s.size())} {}

	[[nodiscard]] value_type get() const { return _id; }
	[[nodiscard]] std::string_view name() const { return IdRegistry::stringFromId(*this); }

	[[nodiscard]] Id withSuffix(u64 suffix) const;
	[[nodiscard]] Id withSuffix(const char* suffix) const;
	[[nodiscard]] Id withPrefix(const char* prefix) const;

	[[nodiscard]] bool isValid() const { return _id != 0; }

	bool operator==(Id id) const { return _id == id._id; }
	bool operator!=(Id id) const { return _id != id._id; }
	bool operator<(Id id) const { return _id < id._id; }
	bool operator>(Id id) const { return _id > id._id; }

	[[nodiscard]] bool equals(const char* s) const;

	operator u64() const { return _id; }

	friend size_t qHash(Id id, size_t seed = 0) noexcept {
		return QT_PREPEND_NAMESPACE(qHash)(id._id, seed);
	}
	friend QDebug operator<<(QDebug dbg, Id id);
private:
	value_type _id = 0;
};
} // namespace Utils