cmake_minimum_required(VERSION 3.15)

set(ARCHE_GUI_NAME "arche-gui")
set(ARCHE_GUI_BUNDLE_NAME "arche-gui-bundle")

project(${ARCHE_GUI_NAME})

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui CACHE PATH "Path to ImGui source")

add_library(IMGUI STATIC 
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

target_include_directories(IMGUI PUBLIC 
	${IMGUI_DIR} 
	${IMGUI_DIR}/backends
)

find_package(OpenGL REQUIRED)

target_link_libraries(IMGUI PUBLIC glfw OpenGL::GL)

add_subdirectory(core)
add_subdirectory(backends/imgui)

add_subdirectory(modules/dockspace)
add_subdirectory(modules/log)
add_subdirectory(modules/metrics)
add_subdirectory(modules/viewport2D)

add_library(arche-gui-bundle INTERFACE)

target_link_libraries(arche-gui-bundle INTERFACE
    arche::gui-core
    arche::gui-backend-imgui
    arche::gui-panel-dockspace
    arche::gui-panel-log
    arche::gui-panel-metrics
    arche::gui-panel-viewport2D
)

add_subdirectory(viewer)


add_library(arche::gui-bundle ALIAS arche-gui-bundle)