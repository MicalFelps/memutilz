project(agent LANGUAGES CXX C)

file(GLOB AGENT_SOURCES CONFIGURE_DEPENDS
    src/dllmain.cpp
    src/pch.cpp
    src/minhook/*.c
    src/minhook/hde/*.c
)

file(GLOB AGENT_HEADERS CONFIGURE_DEPENDS
    hdr/framework.h
    hdr/pch.h
    hdr/minhook/*.h
    src/minhook/*.h
    src/minhook/hde/*.h
)

# Create the DLL
add_library(agent SHARED
    ${AGENT_SOURCES}
    ${AGENT_HEADERS}
)

set_target_properties(agent PROPERTIES
    OUTPUT_NAME "agent"

    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin/${PLATFORM}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin/${PLATFORM}/Release"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin/${PLATFORM}/Debug"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin/${PLATFORM}/Release"
)

# Include directories
target_include_directories(agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/minhook
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minhook
    ${CMAKE_CURRENT_SOURCE_DIR}/src/minhook/hde
)

# Precompiled headers setup
target_precompile_headers(agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/pch.h
)

# Compiler definitions to match MSVC settings
target_compile_definitions(agent PRIVATE
    UNICODE
    _UNICODE
    MEMWATCH_EXPORTS
    _WINDOWS
    _USRDLL
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<PLATFORM_ID:Win32>:WIN32>
)

# Compiler options to match MSVC settings
target_compile_options(agent PRIVATE
    $<$<CONFIG:Debug>:/W3 /sdl /Zi>
    $<$<CONFIG:Release>:/W3 /sdl /O2 /Oi /Gy>
)

# Linker options to match MSVC settings
set_target_properties(agent PROPERTIES
    LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /DEBUG"
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF"
)