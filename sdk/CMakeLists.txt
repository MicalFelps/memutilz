add_library(memutilz_sdk INTERFACE)
add_library(memutilz::sdk ALIAS memutilz_sdk)

target_include_directories(memutilz_sdk
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/memutilz>
)

set_target_properties(memutilz_sdk PROPERTIES LINKER_LANGUAGE C)
