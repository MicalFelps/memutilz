# Check out https://github.com/StableCoder/cmake-scripts.git

add_library(MemutilzWarnings INTERFACE)
add_library(MemutilzOptions INTERFACE)

add_library(Memutilz::Warnings ALIAS MemutilzWarnings)
add_library(Memutilz::Options  ALIAS MemutilzOptions)

option(MEMUTILZ_WARNINGS_AS_ERRORS  "Treat compiler warnings as errors" ON)
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

option(MEMUTILZ_ENABLE_COVERAGE "Enable code coverage" OFF)
if(MEMUTILZ_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(MemutilzOptions INTERFACE -O0 -g --coverage)
        target_link_options(MemutilzOptions INTERFACE --coverage)
    else()
        message(WARNING "Coverage not supported on ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

option(MEMUTILZ_ENABLE_IPO "Enable IPO/LTO when supported" ON)
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
