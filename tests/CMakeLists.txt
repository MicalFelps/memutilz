if(NOT PROJECT_IS_TOP_LEVEL)
    return()
endif()

enable_testing()
include(GoogleTest)

function(add_gtest target)
    set(options AUTO_DISCOVER)
    set(one_value_args TIMEOUT)
    set(multi_value_args SOURCES DEPENDS LABELS)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${options}" "${one_value_args}" "${multi_value_args}")

    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "add_gtest: must specify SOURCES for target '${target}'")
    endif()

    foreach(src ${ARG_SOURCES})
        if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
            message(FATAL_ERROR "add_gtest: source file ${src} does not exist")
        endif()
    endforeach()

    add_executable(${target} ${ARG_SOURCES})

    target_link_libraries(${target}
        PRIVATE
            GTest::gtest
            GTest::gtest_main
            Memutilz::Warnings
            Memutilz::Options
            ${ARG_DEPENDS}
    )

    add_test(NAME ${target} COMMAND ${target})

    if(ARG_TIMEOUT)
        set_tests_properties(${target} PROPERTIES TIMEOUT ${ARG_TIMEOUT})
    endif()

    if(ARG_LABELS)
        set_tests_properties(${target} PROPERTIES LABELS "${ARG_LABELS}")
    endif()

    # Auto-discovery is almost always wanted
    if(NOT DEFINED ARG_AUTO_DISCOVER)
        set(ARG_AUTO_DISCOVER ON)
    endif()

    if(ARG_AUTO_DISCOVER)
        gtest_discover_tests(${target}
            TEST_PREFIX "${target}."
            TEST_SUFFIX ""
            XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/junit/${target}.xml"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
    endif()
endfunction()

add_gtest(test_example
    SOURCES example.cpp
    # No DEPENDS needed for smoke tests
    # LABELS "smoke;basic"
    # TIMEOUT 10
    # AUTO_DISCOVER OFF
)

add_gtest(test_math
    SOURCES math.cpp
    LABELS "smoke;math"
    TIMEOUT 15
)

if(WIN32)
    add_custom_command(TARGET test_example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:test_example>
            $<TARGET_FILE_DIR:test_example>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs for test_example"
    )

    add_custom_command(TARGET test_math POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:test_math>
            $<TARGET_FILE_DIR:test_math>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs for test_math"
    )
endif()
