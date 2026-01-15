function(memutilz_enable_sanitizers target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Target '${target}' does not exist!")
    endif()

    # AddressSanitizer
    if(MEMUTILZ_ENABLE_ASAN)
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag(-fsanitize=address HAS_ASAN)
        if(HAS_ASAN)
            message(STATUS "Enabling AddressSanitizer for target ${target}")
            target_compile_options(${target} PRIVATE -fsanitize=address)
            target_link_options(${target} PRIVATE -fsanitize=address)
        else()
            message(WARNING "ASAN requested but not supported by compiler")
        endif()
    endif()

    # UndefinedBehaviorSanitizer
    if(MEMUTILZ_ENABLE_UBSAN)
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag(-fsanitize=undefined HAS_UBSAN)
        if(HAS_UBSAN)
            message(STATUS "Enabling UndefinedBehaviorSanitizer for target ${target}")
            target_compile_options(${target} PRIVATE -fsanitize=undefined)
            target_link_options(${target} PRIVATE -fsanitize=undefined)
        else()
            message(WARNING "UBSAN requested but not supported by compiler")
        endif()
    endif()

    # Eventually add ThreadSanitizer, LeakSanitizer, etc if desired
endfunction()