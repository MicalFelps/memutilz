include(TargetConfig)

add_library(memutilz_lib)
memutilz_configure_target(memutilz_lib)

set_target_properties(memutilz_lib PROPERTIES
    OUTPUT_NAME "memutilz"
)

target_include_directories(memutilz_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hdr>
        $<INSTALL_INTERFACE:include/memutilz>
)

set(MEMUTILZ_LIB_HEADERS
    hdr/MemorySource.h
)

set(MEMUTILZ_LIB_SOURCES
    src/MemorySource.cpp
)

target_sources(memutilz_lib 
    PRIVATE
        ${MEMUTILZ_LIB_HEADERS}
        ${MEMUTILZ_LIB_SOURCES}
)

target_link_libraries(memutilz_lib
    PRIVATE
        Zydis
)

if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(memutilz_lib PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

install(TARGETS memutilz_lib
    EXPORT memutilzTargets
    LIBRARY DESTINATION    ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION    ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION    ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION   ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT memutilz
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/hdr/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/memutilz/
    COMPONENT memutilz
    FILES_MATCHING PATTERN "*.h"
)