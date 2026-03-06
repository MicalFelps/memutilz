find_package(Doxygen QUIET)
if(NOT DOXYGEN_FOUND)
    message(STATUS "Doxygen not found — skipping docs")
    return()
endif()

include(DoxygenHelpers)

# Optionally specify MAINPAGE

add_doxygen_for_component(
    NAME                "memutilz"
    SOURCE_DIR          "${PROJECT_SOURCE_DIR}/src/"
    TITLE               "Memutilz"
    BRIEF               "Unified UI for Various Debuggers"
    DOXYFILE_TEMPLATE   "${CMAKE_SOURCE_DIR}/docs/Doxyfile.in"
    OUTPUT_DIR          "${CMAKE_BINARY_DIR}/docs"
)

add_doxygen_for_component(
    NAME                "extensions"
    SOURCE_DIR          "${PROJECT_SOURCE_DIR}/extensions/"
    TITLE               "Extensions"
    BRIEF               "Plugins and Debugger Adapters"
    DOXYFILE_TEMPLATE   "${CMAKE_SOURCE_DIR}/docs/Doxyfile.in"
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs"
)

add_doxygen_for_all()
