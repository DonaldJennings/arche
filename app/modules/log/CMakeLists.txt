
set(LOG_PANEL_NAME "arche-gui-panel-log")

add_library(${LOG_PANEL_NAME} STATIC
    Private/LogPanel.cpp
    Public/LogPanel.h
)

target_include_directories(${LOG_PANEL_NAME} PUBLIC
    Public
)

target_link_libraries(${LOG_PANEL_NAME} PUBLIC arche::gui-core arche::gui-backend-imgui)

set_target_properties(${LOG_PANEL_NAME} PROPERTIES FOLDER "GUI/Panels")

add_library(arche::gui-panel-log ALIAS ${LOG_PANEL_NAME})