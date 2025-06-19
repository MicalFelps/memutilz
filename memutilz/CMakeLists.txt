project(memutilz VERSION 1.0.0 LANGUAGES CXX)

# Find QT 6.8.1
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

# Find CppWinRT (if needed)
find_package(Microsoft.Windows.CppWinRT QUIET)

# Source files
file(GLOB MEMUTILZ_SOURCES CONFIGURE_DEPENDS
    src/gui/*.cpp
    src/mem/*.cpp
    src/utilz/*.cpp
    src/main.cpp
)

# Header files
file(GLOB MEMUTILZ_HEADERS CONFIGURE_DEPENDS
    hdr/gui/*.h
    hdr/mem/*.h
    hdr/utilz/*.h
)

# UI and resource files
file(GLOB MEMUTILZ_UI_FILES ui/*.ui)
file(GLOB MEMUTILZ_RESOURCES rsrc/*.qrc)

qt_add_executable(memutilz
    ${MEMUTILZ_SOURCES}
    ${MEMUTILZ_HEADERS}
    ${MEMUTILZ_UI_FILES}
    ${MEMUTILZ_RESOURCES}
    rsrc/memutilz.rc
)

set_target_properties(memutilz PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "memutilz"

    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin/${PLATFORM}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin/${PLATFORM}/Release"

    # For some reason AUTOUIC can't find the ui/ dir on it's own
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

# Include directories
target_include_directories(memutilz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/mem
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/utilz
)

target_compile_options(memutilz PRIVATE
    $<$<CONFIG:Debug>:/W3 /sdl /Zi /MP>
    $<$<CONFIG:Release>:/W3 /sdl /MP /O2 /Oi /Gy>
)

target_compile_definitions(memutilz PRIVATE
    UNICODE
    _UNICODE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Link Qt libraries
target_link_libraries(memutilz PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

if(Microsoft.Windows.CppWinRT_FOUND)
    target_link_libraries(memutilz PRIVATE Microsoft.Windows.CppWinRT)
endif()

set_target_properties(memutilz PROPERTIES
    LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /DEBUG"
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF"
)