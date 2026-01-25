#include <format>
#include <QHash>
#include <QDebug>
#include <xxhash.h>

#include "IdRegistry.h"

namespace Utils {

struct StringHolder {
	StringHolder() = default;

	explicit StringHolder(const char* s, size_t len)
		: str{ s }
		, len{ len }
		, h{ XXH3_64bits(str, len) } {}

	size_t qHash(const StringHolder& sh, size_t seed = 0) noexcept {
		return QT_PREPEND_NAMESPACE(qHash)(sh.h, seed);
	}

	friend bool operator==(const StringHolder& sh1, const StringHolder& sh2) {
		return sh1.h == sh2.h
			&& sh1.len == sh2.len
			&& strcmp(sh1.str, sh2.str) == 0;
	}

	const char* str;
	size_t len;
	u64 h = 0;
};
class IdCache : public QHash<StringHolder, u64>
{
#ifndef MU_ALLOW_STATIC_LEAKS
	~IdCache()
	{
		for (IdCache::iterator it = begin(); it != end(); ++it)
			delete[](const_cast<char*>(it.key().str));
	}
#endif
};

Id IdRegistry::getOrRegister(const char* s, size_t len) {
	if (!s) return Id{};
	if (len == 0) {
		len = strlen(s);
		if (len == 0) return Id{};
	}

	if (auto maybeId = idFromString(s, len)) return *maybeId;
	QWriteLocker lock(&_mutex);
	if (auto maybeId = idFromString(s, len)) return *maybeId; // double check pattern is ok here

	static Id::value_type nextAvailableId = 0x100000;
	Id::value_type nextId = nextAvailableId++;

	StringHolder sh{ s, len };
	char* p = new char[len + 1];
	std::memcpy(p, s, len);
	p[len] = '\0';
	sh.str = p; // required for IdCache to delete properly
	_idFromString.insert(sh, nextId);
	_stringFromId.insert(nextId, sh);
	return Id{ nextId };
}
std::string_view IdRegistry::stringFromId(Id id) {
	QReadLocker lock(&_mutex);
	if (auto it = _stringFromId.constFind(id); it == _stringFromId.cend()) return nullptr;
	const auto& [unused_key, sh] = *it;
	return std::string_view{ sh.str, sh.len };
}
std::optional<Id> IdRegistry::idFromString(const char* s, size_t len) {
	QReadLocker lock(&_mutex);
	StringHolder key{ s, len };
	if (auto it = _idFromString.constFind(key); it == _idFromString.cend()) return std::nullopt;
	const auto& [unused_key, id] = *it;
	return Id{ id };
}
Id Id::withSuffix(u64 suffix) const {
	return Id{ std::format("{}.{}", name(), suffix) };
}
Id Id::withSuffix(const char* suffix) const {
	return Id{ std::format("{}.{}", name(), suffix) };
}
Id Id::withPrefix(const char* prefix) const {
	return Id{ std::format("{}.{}", prefix, name()) };
}
bool Id::equals(const char* s) const {
	std::string_view n = name();
	const char* p = n.data();
	return p && s && strcmp(p, s) == 0;
}
QDebug operator<<(QDebug dbg, Id id) {
	return dbg << id.name();
}
} // namespace Utils