
set(VIEWPORT_2D_PANEL_NAME "arche-gui-panel-viewport2D")

add_library(${VIEWPORT_2D_PANEL_NAME} STATIC
    Private/Viewport2DPanel.cpp
    Private/Register.cpp
    Public/Viewport2DPanel.h
)

target_include_directories(${VIEWPORT_2D_PANEL_NAME} PUBLIC
    Public
)

target_link_libraries(${VIEWPORT_2D_PANEL_NAME} PUBLIC arche::gui-core arche::gui-backend-imgui arche::engine)

set_target_properties(${VIEWPORT_2D_PANEL_NAME} PROPERTIES FOLDER "GUI/Panels")

add_library(arche::gui-panel-viewport2D ALIAS ${VIEWPORT_2D_PANEL_NAME})