#include <QDebug>

#include "Utils/Id.h"

namespace Utils {
Id Id::withSuffix(u64 suffix) const {
    return Id{std::format("{}.{}", name(), suffix)};
}
Id Id::withSuffix(const char* suffix) const {
    return Id{std::format("{}.{}", name(), suffix)};
}
Id Id::withPrefix(const char* prefix) const {
    return Id{std::format("{}.{}", prefix, name())};
}
bool Id::equals(const char* s) const {
    std::string_view n = name();
    const char* p = n.data();
    return p && s && strcmp(p, s) == 0;
}
QDebug operator<<(QDebug dbg, Id id) { return dbg << id.name(); }
}  // namespace Utils
