function(add_doxygen_for_component)
    cmake_parse_arguments(PARSE_ARGV 0 ARG
        ""
        "NAME;SOURCE_DIR;TITLE;BRIEF;MAINPAGE;DOXYFILE_TEMPLATE;OUTPUT_DIR"
        ""
    )

    if(NOT ARG_NAME OR NOT ARG_SOURCE_DIR)
        return()
    endif()

    if(ARG_MAINPAGE AND NOT EXISTS "${ARG_MAINPAGE}")
        message(WARNING "Mainpage '${ARG_MAINPAGE}' not found")
    endif()

    if(NOT EXISTS "${ARG_DOXYFILE_TEMPLATE}")
        message(FATAL_ERROR "Required DOXYFILE_TEMPLATE not found: ${ARG_DOXYFILE_TEMPLATE}")
    endif()

    if(NOT ARG_OUTPUT_DIR)
        set(ARG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs")
    endif()

    set(DOXY_PROJECT_NAME "${ARG_TITLE}")
    set(DOXY_PROJECT_BRIEF "${ARG_BRIEF}")
    set(DOXY_MAINPAGE "${ARG_MAINPAGE}")
    set(DOXY_OUTPUT_DIR "${ARG_OUTPUT_DIR}/${ARG_NAME}")
    string(REPLACE ";" " " DOXY_INPUT "${ARG_SOURCE_DIR}")

    # Create doxygen target
    set(doxyfile_out "${CMAKE_BINARY_DIR}/Doxyfile.${ARG_NAME}")
    configure_file("${ARG_DOXYFILE_TEMPLATE}" "${doxyfile_out}" @ONLY)

    set(target "doxy-${ARG_NAME}")
    doxygen_add_docs("${target}"
        CONFIG_FILE "${doxyfile_out}"
        COMMENT "Generating Doxygen documentation: ${ARG_NAME}"
    )

    set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    set_property(GLOBAL APPEND PROPERTY PROJECT_DOXYGEN_TARGETS "${target}")
    message(STATUS "Created docs target: ${target}")
endfunction()

function(add_doxygen_for_all)
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
endfunction()
