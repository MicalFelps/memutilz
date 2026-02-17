#pragma once
#include <memory>

#ifndef MEMUTILZ_DECLARE_PRIVATE
#define MEMUTILZ_DECLARE_PRIVATE() \
    struct Impl;                   \
    std::unique_ptr<Impl> d;       \
    friend struct Impl;
#endif

#ifndef MEMUTILZ_DECLARE_PUBLIC
#define MEMUTILZ_DECLARE_PUBLIC(classname) \
    classname* _this{nullptr};             \
    Impl(const Impl&) = delete;            \
    Impl& operator=(const Impl&) = delete;
#endif

#ifndef MEMUTILZ_DEFINE_THEME
#define MEMUTILZ_DEFINE_THEME(name, ...)     \
inline const QPalette& name() {              \
    static const QPalette palette = [] {     \
        QPalette p;                          \
        __VA_ARGS__                          \
        return p;                            \
    }();                                     \
    return palette;                          \
}
#endif