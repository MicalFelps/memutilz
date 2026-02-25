find_package(Sanitizers)

if(WIN32)
    set(GUI_TYPE WIN32)
elseif(APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
else()
    set(GUI_TYPE "")
endif()

add_executable(Memutilz ${GUI_TYPE})

set(BINARY_NAME "memutilz")
set_target_properties(Memutilz PROPERTIES
    OUTPUT_NAME ${BINARY_NAME}
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

set(MEMUTILZ_HEADERS
    src/Options.h
    src/Globals.h
    src/gui/Resources.h
    src/gui/Components/RibbonBar.h
    src/gui/Components/ApplicationWidget.h
    src/gui/Components/DockManager.h
    src/gui/Components/RibbonBar/RibbonElementManager.h
    src/gui/Components/RibbonBar/SystemButtonBar.h
    src/gui/Components/RibbonBar/RibbonElementFactory.h
    src/gui/Components/RibbonBar/ButtonGroupWidget.h
    src/gui/Main/MainWindowInternal.h
    src/gui/Main/MainWindow.h
    src/gui/Constants/AppLimits.h
    src/gui/Constants/Ids.h
    src/gui/Utils/DummyWidget.h
)

set(MEMUTILZ_SOURCES
    src/main.cpp
    src/gui/Resources.cpp
    src/gui/Components/DockManager.cpp
    src/gui/Components/ApplicationWidget.cpp
    src/gui/Components/RibbonBar.cpp
    src/gui/Components/RibbonBar/ButtonGroupWidget.cpp
    src/gui/Components/RibbonBar/RibbonElementFactory.cpp
    src/gui/Components/RibbonBar/RibbonElementManager.cpp
    src/gui/Components/RibbonBar/SystemButtonBar.cpp
    src/gui/Main/MainWindow.cpp
    src/gui/Main/MainWindowInternal.cpp
    src/gui/Utils/DummyWidget.cpp
)

set(MEMUTILZ_RESOURCES
    rsrc/memutilz.qrc
    rsrc/memutilz.rc
)

set(MEMUTILZ_FORMS
    src/gui/Components/ApplicationWidget.ui
)

target_sources(Memutilz
    PRIVATE
        ${MEMUTILZ_HEADERS}
        ${MEMUTILZ_SOURCES}
        ${MEMUTILZ_RESOURCES}
        ${MEMUTILZ_FORMS}
)

# Dependencies
if(TARGET SARibbonBar)
    target_compile_options(SARibbonBar PRIVATE -Wno-deprecated-declarations)
endif()

target_link_libraries(Memutilz
    PRIVATE
        KDAB::kddockwidgets
        Memutilz::Warnings
        Memutilz::Options
        Memutilz::Memutilz
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        QWindowKit::Core
        QWindowKit::Widgets
        SARibbonBar::SARibbonBar
)

add_sanitizers(Memutilz)

install(
    TARGETS Memutilz
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Apps
)

if(WIN32)
    set(MEMUTILZ_RUNTIME_DEPS
        kddockwidgets
        SARibbonBar
        MemutilzLib
        QWKCore
        QWKWidgets
    )
    foreach(_dependency IN LISTS MEMUTILZ_RUNTIME_DEPS)
        add_custom_command(TARGET Memutilz POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${_dependency}>
                $<TARGET_FILE_DIR:Memutilz>
        )
    endforeach()

    add_custom_command(TARGET Memutilz POST_BUILD
        COMMAND Qt6::windeployqt
            $<$<CONFIG:Debug>:--debug>
            --no-translations
            --no-quick-import
            --no-system-d3d-compiler
            "$<TARGET_FILE:Memutilz>"
        COMMENT "Deploying Qt runtime with windeployqt"
        VERBATIM
    )

    install(
        TARGETS ${MEMUTILZ_RUNTIME_DEPS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Apps
        OPTIONAL
    )

    qt_generate_deploy_app_script(
        TARGET Memutilz
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
        DEPLOY_TOOL_OPTIONS
            --no-translations
            --no-quick-import
            --no-system-d3d-compiler
    )

    install(SCRIPT ${deploy_script}
        COMPONENT Apps
    )
endif()
