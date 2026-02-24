#include <QHash>

#include <xxhash.h>

#include "Utils/Id.h"
#include "Utils/IdRegistry.h"

namespace Utils {

struct StringHolder {
    StringHolder() = default;

    explicit StringHolder(const char* s, size_t l)
        : str{s}, len{l}, h{XXH3_64bits(str, len)} {}

    friend size_t qHash(const StringHolder& sh, size_t seed = 0) noexcept {
        return QT_PREPEND_NAMESPACE(qHash)(sh.h, seed);
    }

    friend bool operator==(const StringHolder& sh1, const StringHolder& sh2) {
        return sh1.h == sh2.h && sh1.len == sh2.len &&
               strcmp(sh1.str, sh2.str) == 0;
    }

    const char* str;
    size_t len;
    u64 h = 0;
};
class IdCache : public QHash<StringHolder, u64> {
   public:
    ~IdCache() {
        for (IdCache::iterator it = begin(); it != end(); ++it)
            delete[] (const_cast<char*>(it.key().str));
    }
};

QHash<u64, StringHolder> IdRegistry::_stringFromId;
IdCache IdRegistry::_idFromString;
QReadWriteLock IdRegistry::_mutex;

Id IdRegistry::getOrRegister(const char* s, size_t len) {
    if (!s) return Id{};
    if (len == 0) {
        len = strlen(s);
        if (len == 0) return Id{};
    }

    if (auto maybeId = idFromString(s, len)) return *maybeId;
    QWriteLocker lock(&_mutex);
    if (auto maybeId = idFromString(s, len, true))
        return *maybeId;  // double check pattern is ok here

    static Id::value_type nextAvailableId = 0x100000;
    Id::value_type nextId = nextAvailableId++;

    StringHolder sh{s, len};
    char* p = new char[len + 1];
    std::memcpy(p, s, len);
    p[len] = '\0';
    sh.str = p;  // required for IdCache to delete properly
    _idFromString.insert(sh, nextId);
    _stringFromId.insert(nextId, sh);
    return Id{nextId};
}
std::string_view IdRegistry::stringFromId(Id id) {
    QReadLocker lock(&_mutex);
    auto it = _stringFromId.constFind(id);
    if (it == _stringFromId.cend()) return std::string_view{};
    const StringHolder& sh = it.value();
    return std::string_view{sh.str, sh.len};
}
std::optional<Id> IdRegistry::idFromString(const char* s, size_t len,
                                           bool hasWriteLock) {
    if (!hasWriteLock) {
        QReadLocker lock(&_mutex);
    }
    StringHolder key{s, len};
    auto it = _idFromString.constFind(key);
    if (it == _idFromString.cend()) return std::nullopt;
    return Id{it.value()};
}

}  // namespace Utils
