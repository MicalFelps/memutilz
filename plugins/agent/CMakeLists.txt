add_library(agent SHARED)

set(AGENT_HEADERS
    src/pch.h
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

# Include directories
target_include_directories(agent
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/hdr
)

target_precompile_headers(agent
    PRIVATE hdr/pch.h
)

target_include_directories(MemutilzLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(agent PRIVATE
    kernel32
    user32
    psapi
    ntdll
    Memutilz::Warnings
    Memutilz::Options
)

set_target_properties(agent PROPERTIES
    OUTPUT_NAME "agent"
)

install(TARGETS agent
    EXPORT memutilzTargets
    RUNTIME DESTINATION    ${CMAKE_INSTALL_BINDIR}/plugins
    COMPONENT Plugins
)
