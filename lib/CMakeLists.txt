add_library(memutilz_lib)

### Library Properties

set_target_properties(memutilz_lib PROPERTIES
    OUTPUT_NAME "memutilz"
)

# Use C++20
target_compile_features(memutilz_lib PRIVATE cxx_std_20)

# Visible Include Directories
target_include_directories(memutilz_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hdr>
        $<INSTALL_INTERFACE:include>
)

### Sources

set(MEMUTILZ_SOURCES
)

target_sources(memutilz_lib 
    PRIVATE
        ${MEMUTILZ_SOURCES}
)

### Dependancies

# Using FetchContent in root cmake
target_link_libraries(memutilz_lib
    PUBLIC
        zydis::zydis
)

### Windows-Specific

if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(memutilz_lib PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()