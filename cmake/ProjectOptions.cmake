add_library(memutilz_warnings INTERFACE)
add_library(memutilz_options INTERFACE)

add_library(memutilz::warnings ALIAS memutilz_warnings)
add_library(memutilz::options ALIAS memutilz_options)

target_compile_options(MemutilzWarnings INTERFACE
    # MSVC
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /permissive-
    >

    # Clang
    $<$<CXX_COMPILER_ID:Clang>:
        -Wall -Wextra -Wpedantic
        -Wshadow -Wconversion -Wsign-conversion
        -Wnon-virtual-dtor -Wold-style-cast -Wdouble-promotion
    >

    # GCC
    $<$<CXX_COMPILER_ID:GNU>:
        -Wall -Wextra -Wpedantic
        -Wshadow -Wconversion -Wsign-conversion
        -Wduplicated-cond -Wduplicated-branches -Wlogical-op
    >

    # Warnings as errors
    $<$<BOOL:${MEMUTILZ_WARNINGS_AS_ERRORS}>:
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
    >
)

if(MEMUTILZ_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(MemutilzOptions INTERFACE -O0 -g --coverage)
        target_link_options(MemutilzOptions INTERFACE --coverage)
    else()
        message(WARNING "Coverage not supported on ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

if(MEMUTILZ_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set_target_properties(MemutilzOptions
                                PROPERTIES
                                    INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO/LTO not supported: ${ipo_error}")
    endif()
endif()
