add_library(MemutilzLib)
add_library(Memutilz::Memutilz ALIAS MemutilzLib)

set(MEMUTILZ_LIB_HEADERS_PUBLIC
    include/MemorySource.h
)
set(MEMUTILZ_LIB_HEADERS_PRIVATE
)
set(MEMUTILZ_LIB_SOURCES
    src/MemorySource.cpp
)

target_sources(MemutilzLib
    PRIVATE
        ${MEMUTILZ_LIB_HEADERS_PRIVATE}
        ${MEMUTILZ_LIB_SOURCES}
)

target_include_directories(MemutilzLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${LIBRARY_NAME}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(MemutilzLib
    PRIVATE
        Zydis::Zydis
        Memutilz::Warnings
        Memutilz::Options
)

if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(MemutilzLib PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

set(LIBRARY_NAME "memutilz")
set_target_properties(MemutilzLib PROPERTIES
    VERSION     0.0.1
    SOVERSION   0 # API Compatibility
    OUTPUT_NAME ${LIBRARY_NAME}
)

install(TARGETS MemutilzLib
    EXPORT memutilzTargets
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Memutilz
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_NAME}
    COMPONENT Memutilz
    FILES_MATCHING PATTERN "*.h"
)
