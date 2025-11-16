
set(PANEL_NAME "arche-gui-material-browser")

add_library(${PANEL_NAME} STATIC
    Private/MaterialBrowser.cpp
    Public/MaterialBrowser.h
)

target_include_directories(${PANEL_NAME} PUBLIC
    Public
)

target_link_libraries(${PANEL_NAME} PUBLIC arche::gui-core arche::gui-backend-imgui)

set_target_properties(${PANEL_NAME} PROPERTIES FOLDER "GUI/Panels")

add_library(arche::gui-panel-material-browser ALIAS ${PANEL_NAME})