# Qt6 - System-installed only
set(QT_NO_PRIVATE_MODULE_WARNING ON)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
# qt_standard_project_setup()

# Always fetch for consistency
# - QADS
# - SARibbon
if(MEMUTILZ_FETCH_DEPS)
    FetchContent_Declare(
        qtadvanceddocking
        GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
        GIT_TAG 4.4.1
    )
    set(BUILD_EXAMPLES OFF CACHE BOOL "Disable Build QADS Examples" FORCE)

    FetchContent_Declare(
        SARibbon
        GIT_REPOSITORY https://github.com/czyt1988/SARibbon.git
        GIT_TAG        master
    )
    set(SARIBBON_BUILD_EXAMPLES    OFF CACHE BOOL "Disable Build SARibbon Examples" FORCE)
    set(SARIBBON_BUILD_TESTS       OFF CACHE BOOL "Disable SARibbon Tests"    FORCE)
    FetchContent_MakeAvailable(qtadvanceddocking)
    FetchContent_MakeAvailable(SARibbon)
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
)

### Properties

# Use C++20
target_compile_features(memutilz_ui PRIVATE cxx_std_20)

target_include_directories(memutilz_ui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/hdr
)

### Sources

set(MEMUTILZ_HEADERS
    hdr/MainWindow.h
    hdr/Options.h
    hdr/Widgets/IconButton.h
    hdr/Widgets/Metrics.h
    hdr/Widgets/Pages.h
    hdr/Widgets/SideBar.h
)

set(MEMUTILZ_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/Widgets/IconButton.cpp
    src/Widgets/SideBar.cpp
)

set(MEMUTILZ_RESOURCES
    rsrc/MainWindow.qrc
    rsrc/memutilz.rc
)

target_sources(memutilz_ui
    PRIVATE
        ${MEMUTILZ_HEADERS}
        ${MEMUTILZ_SOURCES}
        ${MEMUTILZ_RESOURCES}
)

set_target_properties(memutilz_ui PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

### Dependancies

target_link_libraries(memutilz_ui
    PRIVATE
        memutilz_lib
        qtadvanceddocking-qt6
        SARibbonBar
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

target_compile_definitions(memutilz_ui
    PRIVATE
        UNICODE
        _UNICODE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

### POST-BUILD

if(WIN32)
    get_target_property(_qt6_core_location Qt6::Core IMPORTED_LOCATION_RELEASE)
    if(NOT _qt6_core_location)
        get_target_property(_qt6_core_location Qt6::Core IMPORTED_LOCATION)
    endif()
    get_filename_component(_qt6_bin_dir "${_qt6_core_location}" DIRECTORY)

    if(NOT EXISTS "${_qt6_bin_dir}/windeployqt.exe")
        cmake_path(GET Qt6_DIR PARENT_PATH _qt6_root)
        cmake_path(APPEND _qt6_bin_dir "${_qt6_root}" "bin")
    endif()

    add_custom_command(TARGET memutilz_ui POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E env "PATH=${_qt6_bin_dir}"
            "${_qt6_bin_dir}/windeployqt.exe"
            --no-translations --no-quick-import --no-system-d3d-compiler
            "$<TARGET_FILE:memutilz_ui>"
        COMMENT "[1] Deploying Qt runtime with windeployqt"
        VERBATIM
    )

    add_custom_command(TARGET memutilz_ui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SARibbonBar>
            $<TARGET_FILE_DIR:memutilz_ui>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:qtadvanceddocking-qt6>
            $<TARGET_FILE_DIR:memutilz_ui>
        COMMENT "[2] Moving DLLs to output directory"
        VERBATIM
    )

    add_custom_command(TARGET memutilz_ui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_PDB_FILE:qtadvanceddocking-qt6>
            $<TARGET_FILE_DIR:memutilz_ui>
        COMMENT "[3] Moving PDBs to output directory"
        VERBATIM
    )
endif()