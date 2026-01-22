find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

CPMAddPackage(
    NAME qtadvanceddocking
    GITHUB_REPOSITORY githubuser0xFFFF/Qt-Advanced-Docking-System
    GIT_TAG ${QADS_VER}
    OPTIONS
        "BUILD_EXAMPLES OFF"
)

include(deps/SARibbon)
memutilz_add_saribbon()

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

set(BINARY_NAME "memutilz")

set_target_properties(memutilz_ui PROPERTIES
    OUTPUT_NAME ${BINARY_NAME}
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
    src/gui/Src/BaseWidgets/QConsole.h
    src/gui/Src/Gui/ApplicationWidget.h
    src/gui/Src/Gui/CentralDockingArea.h
    src/gui/Src/Gui/MainWindow.h
    src/gui/Src/Gui/RibbonBar.h
    src/gui/Src/AppLimits.h
    src/Options.h
)

set(MEMUTILZ_UI_SOURCES
    src/gui/Src/BaseWidgets/QConsole.cpp
    src/gui/Src/Gui/ApplicationWidget.cpp
    src/gui/Src/Gui/CentralDockingArea.cpp
    src/gui/Src/Gui/MainWindow.cpp
    src/gui/Src/Gui/RibbonBar.cpp
    src/main.cpp
)

set(MEMUTILZ_UI_RESOURCES
    src/gui/rsrc/memutilz.qrc
    src/gui/rsrc/memutilz.rc
)

target_sources(memutilz_ui
    PRIVATE
        ${MEMUTILZ_UI_HEADERS}
        ${MEMUTILZ_UI_SOURCES}
        ${MEMUTILZ_UI_RESOURCES}
)

target_link_libraries(memutilz_ui
    PRIVATE
        memutilz_lib
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

install(TARGETS memutilz_lib qtadvanceddocking-qt6 SARibbonBar
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT apps
    OPTIONAL 
)

if(WIN32)
    set(MEMUTILZ_UI_RUNTIME_DEPS memutilz_lib qtadvanceddocking-qt6 SARibbonBar)
    foreach(_dependency IN LISTS MEMUTILZ_UI_RUNTIME_DEPS)
        add_custom_command(TARGET memutilz_ui POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${_dependency}>
                $<TARGET_FILE_DIR:memutilz_ui>
        )
    endforeach()

    add_custom_command(TARGET memutilz_ui POST_BUILD
        COMMAND Qt6::windeployqt
            $<$<CONFIG:Debug>:--debug>
            --no-translations
            --no-quick-import
            --no-system-d3d-compiler
            "$<TARGET_FILE:memutilz_ui>"
        COMMENT "Deploying Qt runtime with windeployqt"
        VERBATIM
    )

    qt_generate_deploy_app_script(
        TARGET memutilz_ui
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
        DEPLOY_TOOL_OPTIONS --no-translations --no-quick-import --no-system-d3d-compiler
    )

    install(SCRIPT ${deploy_script}
        COMPONENT apps
    )
endif()