
set(DOCKSPACE_PANEL_NAME "arche-gui-panel-dockspace")

add_library(${DOCKSPACE_PANEL_NAME} STATIC
    Private/DockspacePanel.cpp
    Private/Register.cpp
    Public/DockspacePanel.h
)

target_include_directories(${DOCKSPACE_PANEL_NAME} PUBLIC
    Public
)

target_link_libraries(${DOCKSPACE_PANEL_NAME} PUBLIC arche::gui-core arche::gui-backend-imgui)

set_target_properties(${DOCKSPACE_PANEL_NAME} PROPERTIES FOLDER "GUI/Panels")

add_library(arche::gui-panel-dockspace ALIAS ${DOCKSPACE_PANEL_NAME})