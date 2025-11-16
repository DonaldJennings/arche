
set(PANEL_NAME "arche-gui-shader-browser")

add_library(${PANEL_NAME} STATIC
    Private/ShaderBrowser.cpp
    Public/ShaderBrowser.h
)

target_include_directories(${PANEL_NAME} PUBLIC
    Public
)

target_link_libraries(${PANEL_NAME} PUBLIC arche::gui-core arche::gui-backend-imgui)

set_target_properties(${PANEL_NAME} PROPERTIES FOLDER "GUI/Panels")

add_library(arche::gui-panel-shader-browser ALIAS ${PANEL_NAME})