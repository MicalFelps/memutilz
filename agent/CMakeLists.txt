add_library(agent SHARED)

### Library Properties

set_target_properties(agent PROPERTIES
    OUTPUT_NAME "agent"
    PREFIX ""
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Use C++20
target_compile_features(agent PRIVATE cxx_std_20)

# Include directories
target_include_directories(agent
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hdr>
        $<INSTALL_INTERFACE:include>
)

target_precompile_headers(agent PRIVATE "hdr/pch.h")

### Sources

set(AGENT_HEADERS
    
)

set(AGENT_SOURCES
    src/dllmain.cpp
    src/pch.cpp
)

target_sources(agent
    PRIVATE
        ${AGENT_HEADERS}
        ${AGENT_SOURCES}
)

### Compiler Options

if(MSVC)
    target_compile_options(agent PRIVATE
        /W4           # High warning level
        /permissive-  # Strict conformance
        /MP           # Parallel compilation
    )
endif()

target_compile_definitions(agent
    PRIVATE
        UNICODE
        _UNICODE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

### Dependancies

target_link_libraries(agent PRIVATE
    kernel32
    user32
    psapi
    ntdll
)