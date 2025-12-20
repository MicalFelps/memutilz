# Qt6 - System-installed only
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
# qt_standard_project_setup()

# QADS - always fetch for consistency
if(MEMUTILZ_FETCH_DEPENDENCIES)
    FetchContent_Declare(
        qtadvanceddocking
        GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
        GIT_TAG 4.4.1
    )
    FetchContent_MakeAvailable(qtadvanceddocking)

    # Create Alias
    if(NOT TARGET ads::qtadvanceddocking)
        add_library(ads::qtadvanceddocking ALIAS qtadvanceddocking)
    endif()
    if(NOT TARGET ads::ads)
        add_library(ads::ads ALIAS qtadvanceddocking)
    endif()
endif()

if(NOT TARGET ads::qtadvanceddocking)
    message(FATAL_ERROR "Qt Advanced Docking System (ads::qtadvanceddocking) not found. Enable MEMUTILZ_FETCH_DEPS or install it manually.")
endif()

# Platform Specific

if(WIN32)
    set(GUI_TYPE WIN32)
elseif(APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
else()
    set(GUI_TYPE "")
endif()

### Executable

add_executable(memutilz_ui ${GUI_TYPE})

set_target_properties(memutilz_ui PROPERTIES
    OUTPUT_NAME "memutilz"

    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

### Properties

# Use C++20
target_compile_features(memutilz_ui PRIVATE cxx_std_20)

target_include_directories(memutilz_ui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/hdr
)

### Sources

set(MEMUTILZ_SOURCES
)

target_sources(memutilz_ui
    PRIVATE
        ${MEMUTILZ_SOURCES}
)

### Dependancies

target_link_libraries(memutilz_ui
    PRIVATE
        memutilz_lib
        ads::qtadvanceddocking
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

### Compiler Options

if(MSVC)
    target_compile_options(memutilz_ui PRIVATE
        /W4           # High warning level
        /permissive-  # Strict conformance
        /MP           # Parallel compilation
    )
endif()

target_compile_definitions(memutilz
    PRIVATE
        UNICODE
        _UNICODE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)