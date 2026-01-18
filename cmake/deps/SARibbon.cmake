function(memutilz_add_saribbon)
    if(TARGET SARibbonBar)
        return()
    endif()

    set(SARIBBON_LOCAL_DIR "${MEMUTILZ_DEPENDENCY_DIR}/SARibbon")

    if(MEMUTILZ_USE_LOCAL_DEPS)
        if(EXISTS "${SARIBBON_LOCAL_DIR}/CMakeLists.txt")
            message(STATUS "Using local: SARibbon")
            set(SARIBBON_BUILD_EXAMPLES OFF CACHE BOOL "build the examples" FORCE)
            add_subdirectory(
                ${SARIBBON_LOCAL_DIR}
                "${CMAKE_CURRENT_BINARY_DIR}/_deps/saribbon-build"
            )
            return()
        else()
            message(FATAL_ERROR
                "MEMUTILZ_USE_LOCAL_DEPS is ON, but SARibbon was not found at:"
                "  ${SARIBBON_LOCAL_DIR}"
            )
        endif()
    endif()

    message(STATUS "Fetching via CPM: SARibbon")
    CPMAddPackage(
    NAME SARibbon
    GITHUB_REPOSITORY czyt1988/SARibbon
    GIT_TAG ${SARIBBON_VER}
    OPTIONS
        "SARIBBON_BUILD_EXAMPLES OFF"
    EXCLUDE_FROM_ALL ON
    )
endfunction()