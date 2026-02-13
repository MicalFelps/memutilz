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
    src/gui/Src/Base/CustomPaintToolButton.h
    src/gui/Src/Components/ApplicationWidget.h
    src/gui/Src/Components/CentralDockingArea.h
    src/gui/Src/Components/QConsole.h
    src/gui/Src/Components/RibbonBar.h
    src/gui/Src/Constants/AppLimits.h
    src/gui/Src/Constants/Ids.h
    src/gui/Src/Core/Constants.h
    src/gui/Src/Core/ActionManager/ActionContainer.h
    src/gui/Src/Core/ActionManager/ActionManager.h
    src/gui/Src/Core/ActionManager/Command.h
    src/gui/Src/Main/MainWindow.h
    src/gui/Src/Utils/ButtonPainters.h
    src/gui/Src/Utils/Id.h
    src/gui/Src/Utils/IdRegistry.h
    src/gui/Src/Utils/Types.h
)

set(MEMUTILZ_SOURCES
    src/main.cpp
    src/gui/Src/Base/CustomPaintToolButton.cpp
    src/gui/Src/Components/ApplicationWidget.cpp
    src/gui/Src/Components/CentralDockingArea.cpp
    src/gui/Src/Components/QConsole.cpp
    src/gui/Src/Components/RibbonBar.cpp
    src/gui/Src/Core/ActionManager/ActionContainer.cpp
    src/gui/Src/Core/ActionManager/ActionManager.cpp
    src/gui/Src/Core/ActionManager/Command.cpp
    src/gui/Src/Main/MainWindow.cpp
    src/gui/Src/Utils/ButtonPainters.cpp
    src/gui/Src/Utils/Id.cpp
    src/gui/Src/Utils/IdRegistry.cpp
)

set(MEMUTILZ_RESOURCES
    src/gui/rsrc/memutilz.qrc
    src/gui/rsrc/memutilz.rc
)

set(MEMUTILZ_FORMS
    src/gui/Src/Components/ApplicationWidget.ui
)

target_sources(Memutilz
    PRIVATE
        ${MEMUTILZ_HEADERS}
        ${MEMUTILZ_SOURCES}
        ${MEMUTILZ_RESOURCES}
        ${MEMUTILZ_FORMS}
)

target_link_libraries(Memutilz
    PRIVATE
        ads::qtadvanceddocking-qt6
        SARibbonBar::SARibbonBar
        xxHash::xxHash
        Memutilz::Memutilz
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

add_sanitizers(Memutilz)

install(
    TARGETS Memutilz
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Apps
)

install(
    TARGETS
        qtadvanceddocking-qt6
        SARibbonBar
        Memutilz
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Apps
    OPTIONAL
)

if(WIN32)
    set(MEMUTILZ_RUNTIME_DEPS
        qtadvanceddocking-qt6
        SARibbonBar
        MemutilzLib
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
