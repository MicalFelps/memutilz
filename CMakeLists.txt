cmake_minimum_required(VERSION 3.28)
project(
    "memutilz"
    VERSION 2.0.0
    LANGUAGES C CXX
)

### Global options
option(BUILD_SHARED_LIBS "Build dynamic libraries (.dll/.so) instead of static libs" OFF)
option(MEMUTILZ_BUILD_UI "Build the QT-based executable (UI) alongside the library" ON)
option(MEMUTILZ_FETCH_DEPS "Automatically fetch third-party dependencies if not found on the system" ON)

### Set architecture based on configuration

set(ARCH "" CACHE STRING "Target architecture (x86 or x64)")
set_property(CACHE ARCH PROPERTY STRINGS "x86" "x64")

# Auto-detect architecture if not set
if(NOT ARCH)
    if(DEFINED CMAKE_GENERATOR_PLATFORM)
        if(CMAKE_GENERATOR_PLATFORM MATCHES "^(x64|Win64|AMD64|x86_64)$")
            set(ARCH "x64")
        elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^(Win32|x86)$")
            set(ARCH "x86")
        endif()
    endif()

    # Fallback detection based on pointer size
    if(NOT ARCH)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(ARCH "x64")
        else()
            set(ARCH "x86")
        endif()
    endif()
endif()

# Validate ARCH value
if(NOT ARCH STREQUAL "x86" AND NOT ARCH STREQUAL "x64")
    message(FATAL_ERROR "ARCH must be 'x86' or 'x64', got: '${ARCH}'")
endif()

message(STATUS "Building for architecture: ${ARCH}")

### Output Directories

# Artifacts always go build/binaries/<arch>/
set(_base "${CMAKE_SOURCE_DIR}/build/binaries/${ARCH}")

# Should work fine with whatever generator
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_base}/bin/$<LOWER_CASE:$<CONFIG>>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_base}/lib/$<LOWER_CASE:$<CONFIG>>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${_base}/lib/$<LOWER_CASE:$<CONFIG>>")

### Dependancies

# For easy dependancy management
# Prefer system-installed, fallback to fetch if option enabled
include(FetchContent)

# Zydis (for disassembly)
find_package(Zydis QUIET)
if(NOT Zydis_FOUND AND MEMUTILZ_FETCH_DEPS)
    message(STATUS "Zydis not found; fetching via FetchContent...")
    FetchContent_Declare(
        Zydis
        GIT_REPOSITORY https://github.com/zyantific/zydis.git
        GIT_TAG v4.1.1
    )
    FetchContent_MakeAvailable(Zydis)
endif()

# Make sure it exists now
if(NOT TARGET Zydis AND NOT Zydis_FOUND)
    message(FATAL_ERROR "Zydis library not found. Set MEMUTILZ_FETCH_DEPS=ON or install Zydis system-wide.")
endif()

# Create Alias
if(NOT TARGET zydis::zydis)
    if(TARGET Zydis)
        add_library(zydis::zydis ALIAS Zydis)
    elseif(TARGET Zydis::Zydis)
        add_library(zydis::zydis ALIAS Zydis::Zydis)
    endif()
endif()

### Subdirectories

if(WIN32)
    # add_subdirectory(agent)     # Injectable DLL for windows (only)
else()
    message(STATUS "Skipping agent.dll (Windows only)")
endif()

add_subdirectory(lib)       # Main library (static/shared)

if(MEMUTILZ_BUILD_UI)
    # add_subdirectory(memutilz)  # QT executable (UI app)
endif()