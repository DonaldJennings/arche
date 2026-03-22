set(RT_PANEL_NAME "arche-gui-raytracing-panel")

add_library(${RT_PANEL_NAME} STATIC
    Private/RayTracingPanel.cpp
    Public/RayTracingPanel.h
)

target_include_directories(${RT_PANEL_NAME} PUBLIC
    Public
)

target_link_libraries(${RT_PANEL_NAME} PUBLIC
    arche::gui-core
    arche::gui-backend-imgui
    arche::engine-renderer
)

set_target_properties(${RT_PANEL_NAME} PROPERTIES FOLDER "GUI/Panels")
add_library(arche::gui-raytracing-panel ALIAS ${RT_PANEL_NAME})
