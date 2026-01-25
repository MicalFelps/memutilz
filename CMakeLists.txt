cmake_minimum_required(VERSION 3.28)

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

cmake_policy(SET CMP0135 NEW) # FetchContent timestamp handling

project(
    memutilz
    VERSION 1.0.0
    HOMEPAGE_URL https://github.com/MicalFelps/memutilz
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(Options)
include(TargetConfig)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "No build type selected, defaulting to Debug")
endif()

message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

# ---- dependencies ----

set(MEMUTILZ_DEPENDENCY_DIR "${CMAKE_SOURCE_DIR}/ext")

include(CPM)
include(deps/Versions)

# Zydis
include(deps/Zydis)
memutilz_add_zydis()
add_library(memutilz::zydis ALIAS Zydis)

# xxhash
include(deps/xxhash)
memutilz_add_xxhash()

# ---- subdirectories ----

include(GNUInstallDirs)

if(PROJECT_IS_TOP_LEVEL AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory" FORCE)
endif()

add_subdirectory(src)

if(MEMUTILZ_BUILD_APPS)
    add_subdirectory(apps)
endif()

if(MEMUTILZ_BUILD_PLUGINS)
    add_subdirectory(plugins)
endif()

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

# ---- Installation ----

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/memutilzConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    cmake/memutilzConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/memutilzConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memutilz
)

# Export targets for find_package()
install(
    EXPORT memutilzTargets
    FILE memutilzTargets.cmake
    NAMESPACE memutilz::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memutilz
    COMPONENT memutilz
)

# Install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/memutilzConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/memutilzConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memutilz
    COMPONENT memutilz
)