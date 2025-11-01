set(SIM_CONTROL_PANEL_NAME "arche-gui-panel-simulationcontrol")

add_library(${SIM_CONTROL_PANEL_NAME} STATIC
    Private/SimulationControlPanel.cpp
    Public/SimulationControlPanel.h
)

target_include_directories(${SIM_CONTROL_PANEL_NAME} PUBLIC
    Public
)

target_link_libraries(${SIM_CONTROL_PANEL_NAME} PUBLIC arche::gui-core arche::gui-backend-imgui arche::engine)

set_target_properties(${SIM_CONTROL_PANEL_NAME} PROPERTIES FOLDER "GUI/Panels")

add_library(arche::gui-panel-simulationcontrol ALIAS ${SIM_CONTROL_PANEL_NAME})
