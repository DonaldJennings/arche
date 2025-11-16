cmake_minimum_required(VERSION 3.15)

set(ARCHE_GUI_NAME "arche-gui")
set(ARCHE_GUI_BUNDLE_NAME "arche-gui-bundle")

project(${ARCHE_GUI_NAME})

add_subdirectory(core)
add_subdirectory(backends/imgui)

add_subdirectory(modules/dockspace)
add_subdirectory(modules/log)
add_subdirectory(modules/metrics)
add_subdirectory(modules/viewport3D)
add_subdirectory(modules/world-properties)
add_subdirectory(modules/material-browser)
add_subdirectory(modules/shader-browser)
add_subdirectory(modules/entity-inspector)

add_library(arche-gui-bundle INTERFACE)

target_link_libraries(arche-gui-bundle INTERFACE
    arche::gui-core
    arche::gui-backend-imgui
    arche::gui-panel-dockspace
    arche::gui-panel-log
    arche::gui-panel-metrics
    arche::gui-panel-viewport3D
    arche::gui-panel-world-properties
    arche::gui-panel-material-browser
    arche::gui-panel-shader-browser
    arche::gui-panel-entity-inspector
    arche::imgui
)

add_library(arche::gui-bundle ALIAS arche-gui-bundle)

add_subdirectory(viewer)

