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
