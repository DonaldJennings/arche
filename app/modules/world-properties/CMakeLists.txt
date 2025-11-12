
set(PANEL_NAME "arche-gui-world-properties")

add_library(${PANEL_NAME} STATIC
    Private/WorldProperties.cpp
    Public/WorldProperties.h
)

target_include_directories(${PANEL_NAME} PUBLIC
    Public
)

target_link_libraries(${PANEL_NAME} PUBLIC arche::gui-core arche::gui-backend-imgui)

set_target_properties(${PANEL_NAME} PROPERTIES FOLDER "GUI/Panels")

add_library(arche::gui-panel-world-properties ALIAS ${PANEL_NAME})