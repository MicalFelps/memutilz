add_library(memutilz_sdk_cpp INTERFACE)
add_library(memutilz::sdk_cpp ALIAS memutilz_sdk_cpp)

target_include_directories(memutilz_sdk_cpp
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/memutilz>
)

target_link_libraries(memutilz_sdk_cpp
    INTERFACE
        memutilz::sdk
        memutilz::common
)
