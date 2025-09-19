project(agent LANGUAGES CXX C)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/minhook)

file(GLOB AGENT_SOURCES CONFIGURE_DEPENDS
    src/coms/coms.cpp
    src/dllmain.cpp
    src/pch.cpp
)

file(GLOB AGENT_HEADERS CONFIGURE_DEPENDS
    hdr/coms/coms.h
    hdr/framework.h
    hdr/pch.h
)

# Create the DLL
add_library(agent SHARED
    ${AGENT_SOURCES}
    ${AGENT_HEADERS}
)

set_target_properties(agent PROPERTIES
    OUTPUT_NAME "agent"

    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SOLUTION_DIR}/${PLATFORM}/bin/Debug/${CMAKE_PROJECT_NAME}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SOLUTION_DIR}/${PLATFORM}/bin/Release/${CMAKE_PROJECT_NAME}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${SOLUTION_DIR}/${PLATFORM}/bin/Debug/${CMAKE_PROJECT_NAME}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${SOLUTION_DIR}/${PLATFORM}/bin/Release/${CMAKE_PROJECT_NAME}"
)

# Include directories
target_include_directories(agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr
)

# Precompiled headers setup
target_precompile_headers(agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/pch.h
)

target_link_libraries(agent PRIVATE minhook)

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