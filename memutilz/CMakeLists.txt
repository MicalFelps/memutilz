# Qt6 - System-installed only
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
# qt_standard_project_setup()

# QADS - always fetch for consistency
if(MEMUTILZ_FETCH_DEPS)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        qtadvanceddocking
        GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
        GIT_TAG 4.4.1
    )
    FetchContent_MakeAvailable(qtadvanceddocking)
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
)

set(MEMUTILZ_SOURCES
    src/main.cpp
    src/MainWindow.cpp
)

set(MEMUTILZ_RESOURCES
    rsrc/MainWindow.qrc
    rsrc/MainWindow.ui
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

    # Helps AUTOUIC find the ui file
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/rsrc"
)

### Dependancies

target_link_libraries(memutilz_ui
    PRIVATE
        memutilz_lib
        qtadvanceddocking-qt6
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
    get_target_property(_qt6_qmake_location Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt6_bin_dir "${_qt6_qmake_location}" DIRECTORY)
    
    # Run windeployqt
    add_custom_command(TARGET memutilz_ui POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt6_bin_dir}" 
            "${_qt6_bin_dir}/windeployqt.exe"
            "$<TARGET_FILE:memutilz_ui>"
        COMMENT "Running windeployqt on Windows..."
    )
endif()