add_library(agent SHARED)
memutilz_configure_target(agent)

set_target_properties(agent PROPERTIES
    OUTPUT_NAME "agent"
)

# Include directories
target_include_directories(agent
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/hdr
)

target_precompile_headers(agent
    PRIVATE hdr/pch.h
)

set(AGENT_HEADERS
    hdr/pch.h
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

target_link_libraries(agent PRIVATE
    kernel32
    user32
    psapi
    ntdll
)

install(TARGETS agent
    EXPORT memutilzTargets
    RUNTIME DESTINATION    ${CMAKE_INSTALL_BINDIR}/plugins
    COMPONENT plugins
)