function(memutilz_add_constexpr_xxh3)
    if(TARGET xxHash::constexpr-xxh3)
        return()
    endif()

    CPMAddPackage(
        NAME constexpr-xxh3
        GIT_REPOSITORY https://github.com/chys87/constexpr-xxh3.git
        GIT_TAG ${CONSTEXPR_XXH3_VER}
        DOWNLOAD_ONLY TRUE
    )

    if(NOT constexpr-xxh3_ADDED)
        message(FATAL_ERROR "Failed to download constexpr-xxh3")
    endif()

    set(CONSTEXPR_XXH3_ROOT "${constexpr-xxh3_SOURCE_DIR}")

    add_library(constexpr-xxh3_header_only INTERFACE)
    
    target_include_directories(constexpr-xxh3_header_only
        INTERFACE
            $<BUILD_INTERFACE:${CONSTEXPR_XXH3_ROOT}>
            $<INSTALL_INTERFACE:include>
    )

    target_compile_features(constexpr-xxh3_header_only INTERFACE cxx_std_20)

    add_library(xxHash::constexpr-xxh3 ALIAS constexpr-xxh3_header_only)
endfunction()