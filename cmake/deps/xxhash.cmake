function(memutilz_add_xxhash)
    if(TARGET xxHash::xxHash)
        return()
    endif()

    CPMAddPackage(
        NAME xxHash
        GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
        GIT_TAG        ${XXHASH_VER}
        DOWNLOAD_ONLY  TRUE     # we do custom target setup below
    )

    if(NOT xxHash_ADDED)
        message(FATAL_ERROR "Failed to download xxHash")
    endif()

    set(XXH_ROOT "${xxHash_SOURCE_DIR}")

    add_library(xxHash_header_only INTERFACE)

    target_include_directories(xxHash_header_only
        INTERFACE
            $<BUILD_INTERFACE:${XXH_ROOT}>
            $<INSTALL_INTERFACE:include>
    )

    target_compile_definitions(xxHash_header_only
        INTERFACE
            XXH_INLINE_ALL
            # XXH_PRIVATE_API
    )

    add_library(xxHash::xxHash ALIAS xxHash_header_only)
endfunction()