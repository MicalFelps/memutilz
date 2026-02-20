if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CPM_USE_LOCAL_PACKAGES ON
    CACHE BOOL "Prefer locally installed packages via find_package")

if(NOT DEFINED CPM_SOURCE_CACHE)
    if(WIN32)
        set(CPM_SOURCE_CACHE_DIR "$ENV{USERPROFILE}/.cache/CPM")
    else()
        set(CPM_SOURCE_CACHE_DIR "$ENV{HOME}/.cache/CPM")
    endif()

    set(CPM_SOURCE_CACHE "${CPM_SOURCE_CACHE_DIR}"
        CACHE PATH "Shared CPM download cache")
endif()

include(CPM)
include(GlobalOptions)
include(Versions)

# Global

CPMAddPackage(
    NAME Zydis
    GITHUB_REPOSITORY zyantific/zydis
    GIT_TAG ${ZYDIS_BRANCH}
    EXCLUDE_FROM_ALL TRUE
)

CPMAddPackage(
    NAME xxHash
    GITHUB_REPOSITORY Cyan4973/xxHash
    GIT_TAG ${XXHASH_BRANCH}
    DOWNLOAD_ONLY YES
)
add_library(xxHash::xxHash INTERFACE IMPORTED GLOBAL)
target_include_directories(xxHash::xxHash
    INTERFACE
        $<BUILD_INTERFACE:${xxHash_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)
target_compile_definitions(xxHash::xxHash
    INTERFACE XXH_INLINE_ALL # XXH_PRIVATE_API
)

# APPS

if(MEMUTILZ_BUILD_APPS)
    set(QT_NO_PRIVATE_MODULE_WARNING ON CACHE BOOL "Disable Qt private module warnings")
    find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets)

    CPMAddPackage(
        NAME SARibbon
        GITHUB_REPOSITORY MicalFelps/SARibbon
        GIT_TAG ${SARIBBON_BRANCH}
        OPTIONS
            "SARIBBON_BUILD_EXAMPLES OFF"
        EXCLUDE_FROM_ALL ON
    )

    CPMAddPackage(
        NAME QWindowKit
        GITHUB_REPOSITORY stdware/qwindowkit
        GIT_TAG ${QWINDOWKIT_BRANCH}
        EXCLUDE_FROM_ALL ON
    )

    CPMAddPackage(
        NAME KDDockWidgets
        GITHUB_REPOSITORY KDAB/KDDockWidgets
        GIT_TAG ${KDDOCKWIDGETS_BRANCH}
        OPTIONS
            "KDDockWidgets_EXAMPLES OFF"
        EXCLUDE_FROM_ALL ON
    )
endif()

# TESTING

if(BUILD_TESTING)
    CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG ${GTEST_TAG}
        OPTIONS
            "INSTALL_GTEST OFF"
            "gtest_force_shared_crt ON"
        EXCLUDE_FROM_ALL TRUE
    )
endif()
