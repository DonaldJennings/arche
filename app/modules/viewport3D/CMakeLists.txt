
set(PANEL_NAME "arche-gui-panel-viewport3D")

# GLOB SOURCE_FILES
file(GLOB_RECURSE SOURCE_FILES
    "Private/*.cpp"
    "Public/*.h"
)

add_library(${PANEL_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PANEL_NAME} PUBLIC
    Public
)

target_link_libraries(${PANEL_NAME} PUBLIC arche::gui-core arche::gui-backend-imgui arche::engine)

set_target_properties(${PANEL_NAME}  PROPERTIES FOLDER "GUI/Panels")

add_library(arche::gui-panel-viewport3D ALIAS ${PANEL_NAME})