function(memutilz_enable_coverage target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Target '${target}' does not exist!")
    endif()

    if(NOT MEMUTILZ_ENABLE_COVERAGE)
        return()
    endif()

    # Only for supported compilers (GCC/Clang)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(WARNING "Code coverage is only supported on GCC/Clang")
        return()
    endif()

    # Only for Debug builds
    if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Coverage disabled for non-Debug build")
        return()
    endif()

    # Compiler flags
    target_compile_options(${target} PRIVATE --coverage -O0 -g)
    target_link_options(${target} PRIVATE --coverage)

    message(STATUS "Code coverage enabled for target ${target}")
endfunction()