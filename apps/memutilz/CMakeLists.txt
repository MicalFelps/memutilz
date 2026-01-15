find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

CPMAddPackage(
    NAME qtadvanceddocking
    GITHUB_REPOSITORY githubuser0xFFFF/Qt-Advanced-Docking-System
    GIT_TAG 4.4.1
    OPTIONS
        "BUILD_EXAMPLES OFF"
)

CPMAddPackage(
    NAME SARibbon
    GITHUB_REPOSITORY czyt1988/SARibbon
    GIT_TAG master
    OPTIONS
        "SARIBBON_BUILD_EXAMPLES OFF"
)

if(WIN32)
    set(GUI_TYPE WIN32)
elseif(APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
else()
    set(GUI_TYPE "")
endif()

include(TargetConfig)

add_executable(memutilz_ui ${GUI_TYPE})
memutilz_configure_target(memutilz_ui)

set_target_properties(memutilz_ui PROPERTIES
    OUTPUT_NAME "memutilz"
)

set_target_properties(memutilz_ui PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_include_directories(memutilz_ui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/hdr
)

set(MEMUTILZ_UI_HEADERS
    hdr/MainWindow.h
    hdr/Options.h
)

set(MEMUTILZ_UI_SOURCES
    src/main.cpp
    src/MainWindow.cpp
)

set(MEMUTILZ_UI_RESOURCES
    rsrc/MainWindow.qrc
    rsrc/memutilz.rc
)

target_sources(memutilz_ui
    PRIVATE
        ${MEMUTILZ_UI_HEADERS}
        ${MEMUTILZ_UI_SOURCES}
        ${MEMUTILZ_UI_RESOURCES}
)

target_link_libraries(memutilz_ui
    PRIVATE
        qtadvanceddocking-qt6
        SARibbonBar
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

install(
    TARGETS memutilz_ui
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT apps
)

if(WIN32)
    get_target_property(_qt6_qmake_location Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt6_bin_dir "${_qt6_qmake_location}" DIRECTORY)
    set(_windeployqt "${_qt6_bin_dir}/windeployqt.exe")

    if(NOT EXISTS "${_windeployqt}")
        message(FATAL_ERROR "windeployqt not found at ${_windeployqt}")
    endif()

    add_custom_command(TARGET memutilz_ui POST_BUILD
        COMMAND "${_windeployqt}"
            --no-translations
            --no-quick-import
            --no-system-d3d-compiler
            "$<TARGET_FILE:memutilz_ui>"
        COMMENT "Deploying Qt runtime with windeployqt"
        VERBATIM
    )
endif()
