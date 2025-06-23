project(memutilz VERSION 1.0.0 LANGUAGES CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/Qt-Advanced-Docking-System)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/capstone)

# Find QT 6.8.1
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

# Find CppWinRT (if needed)
find_package(Microsoft.Windows.CppWinRT QUIET)

# Source files
set(MEMUTILZ_SOURCES
    src/main.cpp
    src/gui/MainWindow.cpp
    src/gui/Ui_MainWindow.cpp
    src/gui/MemoryView/AbstractMemoryView.cpp
    src/gui/MemoryView/AssemblyView.cpp
    src/gui/MemoryView/Hexview.cpp
    src/mem/Memdump.cpp
    src/mem/Meminfo.cpp
    src/mem/Process.cpp
    src/utilz/privilege.cpp
)

# Header files
set(MEMUTILZ_HEADERS
    hdr/gui/common.h
    hdr/gui/MainWindow.h
    hdr/gui/Ui_MainWindow.h
    hdr/gui/MemoryView/AbstractMemoryView.h
    hdr/gui/MemoryView/AssemblyView.h
    hdr/gui/MemoryView/Hexview.h
    hdr/mem/common.h
    hdr/mem/constants.h
    hdr/mem/Exception.h
    hdr/mem/Handle.h
    hdr/mem/Memdump.h
    hdr/mem/Meminfo.h
    hdr/mem/Process.h
    hdr/utilz/privilege.h
)

# resource files
file(GLOB MEMUTILZ_RESOURCES rsrc/*.qrc)

qt_add_executable(memutilz
    ${MEMUTILZ_SOURCES}
    ${MEMUTILZ_HEADERS}
    ${MEMUTILZ_RESOURCES}
    rsrc/memutilz.rc
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set_target_properties(memutilz PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "memutilz"

    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SOLUTION_DIR}/${PLATFORM}/bin/Debug/${CMAKE_PROJECT_NAME}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SOLUTION_DIR}/${PLATFORM}/bin/Release/${CMAKE_PROJECT_NAME}"

    # For some reason AUTOUIC can't find the ui/ dir on it's own
    # AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

# Include directories
target_include_directories(memutilz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/gui/MemoryView
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/mem
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/utilz
)

target_compile_options(memutilz PRIVATE
    $<$<CONFIG:Debug>:/W3 /sdl /Zi /MP /EHsc>
    $<$<CONFIG:Release>:/W3 /sdl /MP /O2 /Oi /Gy /EHsc>
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
    qtadvanceddocking-qt6
    capstone
)

if(Microsoft.Windows.CppWinRT_FOUND)
    target_link_libraries(memutilz PRIVATE Microsoft.Windows.CppWinRT)
endif()

set_target_properties(memutilz PROPERTIES
    LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /DEBUG"
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF"
)