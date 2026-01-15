set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(CompilerWarnings)
include(Coverage)
include(IPO)
include(Sanitizers)

function(memutilz_configure_target target)
    target_compile_features(${target} PRIVATE cxx_std_20)
    memutilz_set_warnings(${target})
    memutilz_enable_ipo(${target})
    memutilz_enable_sanitizers(${target})
    memutilz_enable_coverage(${target})
endfunction()