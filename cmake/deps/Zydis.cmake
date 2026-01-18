function(memutilz_add_zydis)
    if(TARGET Zydis)
        return()
    endif()

    set(ZYDIS_LOCAL_DIR "${MEMUTILZ_DEPENDENCY_DIR}/Zydis")

    if(MEMUTILZ_USE_LOCAL_DEPS)
        if(EXISTS "${ZYDIS_LOCAL_DIR}/CMakeLists.txt")
            message(STATUS "Using local: Zydis")
            set(SARIBBON_BUILD_EXAMPLES OFF)
            add_subdirectory(${ZYDIS_LOCAL_DIR})
            return()
        else()
            message(FATAL_ERROR
                "MEMUTILZ_USE_LOCAL_DEPS is ON, but Zydis was not found at:"
                "  ${ZYDIS_LOCAL_DIR}"
            )
        endif()
    endif()

    message(STATUS "Fetching via CPM: Zydis")
    CPMAddPackage(
        NAME Zydis
        GITHUB_REPOSITORY zyantific/zydis
        GIT_TAG ${ZYDIS_VER}
        EXCLUDE_FROM_ALL TRUE
    )
endfunction()