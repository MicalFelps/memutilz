add_library(memutilz_common)
add_library(memutilz::common ALIAS memutilz_common)

set(COMMON_HEADERS
)

set(COMMON_SOURCES
)

target_sources(memutilz_common
    ${COMMON_HEADERS}
    ${COMMON_SOURCES}
)

target_include_directories(memutilz_common
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(memutilz_common
    PRIVATE
        memutilz::warnings
        memutilz::options
)

# if(WIN32 AND BUILD_SHARED_LIBS)
#     set_target_properties(memutilz_common PROPERTIES
#         WINDOWS_EXPORT_ALL_SYMBOLS ON
#     )
# endif()

set(LIBRARY_NAME "common")
set_target_properties(memutilz_common PROPERTIES
    OUTPUT_NAME ${LIBRARY_NAME}
)

install(TARGETS memutilz_common
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT runtime
)
