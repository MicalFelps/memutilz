add_subdirectory(bootstrap)
add_subdirectory(core)
add_subdirectory(memutilz)

if(NOT MEMUTILZ_BUILD_HEADLESS)
    add_subdirectory(gui)
endif()
