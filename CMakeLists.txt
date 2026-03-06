cmake_minimum_required(VERSION 3.28)

# Policies
cmake_policy(SET CMP0077 NEW) # respect the user's override
cmake_policy(SET CMP0135 NEW) # timestamps are set during extraction

# Takes care of potential symlinks
file(REAL_PATH "${CMAKE_SOURCE_DIR}" SRC_DIR)
file(REAL_PATH "${CMAKE_BINARY_DIR}" BIN_DIR)

# Prevent in-source builds
if(SRC_DIR STREQUAL BIN_DIR)
    message(FATAL_ERROR
        "In-source builds aren't allowed\n"
        "Create a separate build directory and run cmake from there\n"
    )
endif()

project(memutilz
    VERSION 0.0.2
    DESCRIPTION "Unified UI for various debuggers"
    HOMEPAGE_URL https://github.com/MicalFelps/memutilz
    LANGUAGES CXX C
)

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmake"
    "${CMAKE_SOURCE_DIR}/3rdParty/sanitizers-cmake/cmake"
)

# Configuration Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

option(MEMUTILZ_WARNINGS_AS_ERRORS "Treat compiler warnings as errors for memutilz" ON)
option(MEMUTILZ_ENABLE_COVERAGE "Enable code coverage instrumentation for memutilz" OFF)
option(MEMUTILZ_ENABLE_IPO "Enable interprocedural optimization / LTO when supported" ON)

option(MEMUTILZ_BUILD_HEADLESS "Build headless version of memutilz" OFF)
option(MEMUTILZ_BUILD_ADAPTERS "Build built-in debug adapters" OFF)
option(MEMUTILZ_BUILD_PLUGINS "Build built-in plugins" OFF)
option(MEMUTILZ_BUILD_DOCS "Generate doxygen documentation for memutilz" ON)
option(BUILD_TESTING "Build tests (can disable per-target)" OFF)

# Early Includes
include(ProjectOptions)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# CPM
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

# Global Settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "No build type selected, defaulting to Debug")
endif()

message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

# ---- subdirectories ----

if(PROJECT_IS_TOP_LEVEL AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(
        CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install"
        CACHE PATH "Installation Directory" FORCE
    )
endif()

add_subdirectory(sdk)
add_subdirectory(sdk_cpp)
add_subdirectory(common)
add_subdirectory(src)
add_subdirectory(extensions)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(MEMUTILZ_BUILD_DOCS)
    add_subdirectory(docs)
endif()

# ---- Installation ----

# Generate and install config files
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/memutilzConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/memutilzConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memutilz
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/memutilzConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/memutilzConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/memutilzConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memutilz
    COMPONENT dev
)

# Install sdk headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sdk/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/memutilz
    FILES_MATCHING
        PATTERN "*.h"
    PATTERN "CMakeLists.txt" EXCLUDE
    COMPONENT dev
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sdk_cpp/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/memutilz
    FILES_MATCHING
        PATTERN "*.h"
    PATTERN "CMakeLists.txt" EXCLUDE
    COMPONENT dev
)

# Export memutilz::sdk and memutilz::sdk_cpp
install(TARGETS memutilz_sdk memutilz_sdk_cpp
    EXPORT memutilz_targets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/memutilz
    COMPONENT dev
)

install(EXPORT memutilz_targets
    FILE memutilzTargets.cmake
    NAMESPACE memutilz::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memutilz
    COMPONENT dev
)
