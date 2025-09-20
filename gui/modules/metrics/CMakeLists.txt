
set(METRICS_PANEL_NAME "arche-gui-panel-metrics")

add_library(${METRICS_PANEL_NAME} STATIC
    Private/MetricsPanel.cpp
    Private/Register.cpp
    Public/MetricsPanel.h
)

target_include_directories(${METRICS_PANEL_NAME} PUBLIC
    Public
)

target_link_libraries(${METRICS_PANEL_NAME} PUBLIC arche::gui-core arche::gui-backend-imgui)

set_target_properties(${METRICS_PANEL_NAME} PROPERTIES FOLDER "GUI/Panels")

add_library(arche::gui-panel-metrics ALIAS ${METRICS_PANEL_NAME})