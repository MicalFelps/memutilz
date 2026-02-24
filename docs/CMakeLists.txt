include(${CMAKE_SOURCE_DIR}/cmake/DoxygenUtils.cmake)

add_doxygen_for_libraries(
    SOURCE_DIRS
        "${CMAKE_SOURCE_DIR}/src/lib/src"
    TITLE       "Main Libraries"
    BRIEF       "Core utilities for GUI"
    MAINPAGE    "memutilz_lib.md"
    DOXYFILE_TEMPLATE "${CMAKE_SOURCE_DIR}/docs/Doxyfile.in"
    OUTPUT_BASE_DIR "${CMAKE_BINARY_DIR}/docs"
)

add_doxygen_for_application(
    NAME        "memutilz"
    SOURCE_DIR  "${PROJECT_SOURCE_DIR}/apps/memutilz/src"
    TITLE       "Memutilz"
    BRIEF       "Main GUI Application"
    MAINPAGE    "memutilz.md"
    DOXYFILE_TEMPLATE "${CMAKE_SOURCE_DIR}/docs/Doxyfile.in"
    OUTPUT_BASE_DIR "${CMAKE_BINARY_DIR}/docs"
)

finalize_doxygen_targets()
