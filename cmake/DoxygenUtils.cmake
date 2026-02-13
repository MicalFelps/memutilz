function(_add_one_doxygen_target)
    cmake_parse_arguments(PARSE_ARGV 0 ARG ""
        "TARGET_SUFFIX;TITLE;BRIEF;MAINPAGE;DOXYFILE_TEMPLATE;OUTPUT_BASE_DIR;TARGET_OUT_VAR"
        "INPUT_DIRS"
    )

    find_package(Doxygen QUIET)
    if(NOT DOXYGEN_FOUND)
        if(NOT MEMUTILZ_DOXYGEN_NOT_FOUND_MESSAGE_SHOWN)
            message(STATUS "Doxygen not found — skipping documentation targets")
            set(MEMUTILZ_DOXYGEN_NOT_FOUND_MESSAGE_SHOWN TRUE CACHE INTERNAL "")
        endif()
        set(${ARG_TARGET_OUT_VAR} "" PARENT_SCOPE)
        return()
    endif()

    if(NOT ARG_OUTPUT_BASE_DIR)
        set(ARG_OUTPUT_BASE_DIR "${CMAKE_BINARY_DIR}/docs")
    endif()

    if(NOT ARG_INPUT_DIRS)
        set(${ARG_TARGET_OUT_VAR} "" PARENT_SCOPE)
        return()
    endif()

    set(missing "")
    set(input_dirs ${ARG_INPUT_DIRS})
    foreach(p IN LISTS input_dirs)
        if(NOT EXISTS "${p}")
            list(APPEND missing "${p}")
        endif()
    endforeach()
    if(missing)
        message(STATUS "Docs: Skipping doxy-${ARG_TARGET_SUFFIX}: missing paths ${missing}")
        return()
    endif()

    set(mainpage_path "")
    if(ARG_MAINPAGE)
        foreach(base IN ITEMS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}")
            if(EXISTS "${base}/${ARG_MAINPAGE}")
                set(mainpage_path "${base}/${ARG_MAINPAGE}")
                break()
            endif()
        endforeach()
        if(NOT mainpage_path AND EXISTS "${ARG_MAINPAGE}")
            set(mainpage_path "${ARG_MAINPAGE}")
        endif()
        if(mainpage_path)
            list(APPEND input_dirs ${mainpage_path})
        else()
            message(WARNING "Mainpage '${ARG_MAINPAGE}' not found for target ${ARG_TARGET_SUFFIX}")
        endif()
    endif()

    # Generate Template from Doxyfile.in file
    if(NOT EXISTS "${ARG_DOXYFILE_TEMPLATE}")
        message(FATAL_ERROR "Required DOXYFILE_TEMPLATE not found: ${ARG_DOXYFILE_TEMPLATE}")
    endif()

    set(DOXY_PROJECT_NAME "${ARG_TITLE}")
    set(DOXY_PROJECT_BRIEF "${ARG_BRIEF}")
    set(DOXY_MAINPAGE "${mainpage_path}")
    set(DOXY_OUTPUT_DIR "${ARG_OUTPUT_BASE_DIR}/${ARG_TARGET_SUFFIX}")
    string(REPLACE ";" " " DOXY_INPUT "${input_dirs}")

    # Create doxygen target
    set(doxyfile_out "${CMAKE_BINARY_DIR}/Doxyfile.${ARG_TARGET_SUFFIX}")
    configure_file("${ARG_DOXYFILE_TEMPLATE}" "${doxyfile_out}" @ONLY)

    set(target_name "doxy-${ARG_TARGET_SUFFIX}")
    doxygen_add_docs("${target_name}"
        CONFIG_FILE "${doxyfile_out}"
        COMMENT "Generating Doxygen documentation: ${ARG_TITLE}"
    )

    set_target_properties(${target_name} PROPERTIES EXCLUDE_FROM_ALL TRUE)

    set(${ARG_TARGET_OUT_VAR} "${target_name}" PARENT_SCOPE)
endfunction()
function(_add_doxygen_for_component)
    cmake_parse_arguments(PARSE_ARGV 0 ARG
        ""
        "TYPE;NAME;SOURCE_DIR;TITLE;BRIEF;MAINPAGE;DOXYFILE_TEMPLATE;OUTPUT_BASE_DIR"
        ""
    )

    if(NOT ARG_NAME OR NOT ARG_SOURCE_DIR OR NOT EXISTS "${ARG_SOURCE_DIR}")
        if(ARG_NAME)
            message(STATUS "Skipping docs for ${ARG_TYPE} '${ARG_NAME}' (invalid/missing SOURCE_DIR)")
        endif()
        return()
    endif()

    set(title "${ARG_TITLE}")
    if(NOT title)
        set(title "${ARG_NAME}")
    endif()

    _add_one_doxygen_target(
        TARGET_SUFFIX     "${ARG_NAME}"
        INPUT_DIRS        "${ARG_SOURCE_DIR}"
        TITLE             "${title}"
        BRIEF             "${ARG_BRIEF}"
        MAINPAGE          "${ARG_MAINPAGE}"
        DOXYFILE_TEMPLATE "${ARG_DOXYFILE_TEMPLATE}"
        OUTPUT_BASE_DIR   "${ARG_OUTPUT_BASE_DIR}"
        TARGET_OUT_VAR    target
    )

    if(target)
        set_property(GLOBAL APPEND PROPERTY PROJECT_DOXYGEN_TARGETS "${target}")
        message(STATUS "Created docs target: ${target}")
    endif()
endfunction()

function(add_doxygen_for_application)
    _add_doxygen_for_component(TYPE "application" ${ARGV})
endfunction()
function(add_doxygen_for_plugin)
    _add_doxygen_for_component(TYPE "plugin" ${ARGV})
endfunction()
function(add_doxygen_for_libraries)
    cmake_parse_arguments(PARSE_ARGV 0 ARG
        ""
        "TITLE;BRIEF;MAINPAGE;DOXYFILE_TEMPLATE;OUTPUT_BASE_DIR"
        "SOURCE_DIRS"
    )

    if(NOT ARG_SOURCE_DIRS)
        return()
    endif()

    set(title "${ARG_TITLE}")
    if(NOT title)
        set(title "Libraries")
    endif()

    _add_one_doxygen_target(
        TARGET_SUFFIX          "libs"
        INPUT_DIRS      "${ARG_SOURCE_DIRS}"
        TITLE           "${title}"
        BRIEF           "${ARG_BRIEF}"
        MAINPAGE        "${ARG_MAINPAGE}"
        DOXYFILE_TEMPLATE "${ARG_DOXYFILE_TEMPLATE}"
        OUTPUT_BASE_DIR "${ARG_OUTPUT_BASE_DIR}"
        TARGET_OUT_VAR  target
    )

    if(target)
        set_property(GLOBAL APPEND PROPERTY PROJECT_DOXYGEN_TARGETS "${target}")
        message(STATUS "Created docs target: ${target}")
    endif()
endfunction()

function(finalize_doxygen_targets)
    get_property(all_targets GLOBAL PROPERTY PROJECT_DOXYGEN_TARGETS)

    if(NOT all_targets)
        return()
    endif()

    if(TARGET doxy-all)
        message(WARNING "doxy-all already exists")
        return()
    endif()

    add_custom_target(doxy-all
        DEPENDS ${all_targets}
        COMMENT "Generate all Doxygen documentation"
        VERBATIM
    )

    add_custom_target(docs
        DEPENDS doxy-all
        COMMENT "Alias for doxy-all"
        VERBATIM
    )

    if(NOT MEMUTILZ_BUILD_DOCS)
        set_target_properties(doxy-all docs PROPERTIES
            EXCLUDE_FROM_ALL TRUE
        )
    endif()
endfunction()
