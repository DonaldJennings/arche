set(IMGUI_BACKEND_NAME "arche-gui-backend-imgui")

add_library(${IMGUI_BACKEND_NAME} STATIC
    Private/ImGuiBackend.cpp
    Public/ImGuiBackend.h
)

target_include_directories(${IMGUI_BACKEND_NAME} PUBLIC
    Public
    ${CMAKE_SOURCE_DIR}/gui/imgui
)


# Link dependencies
target_link_libraries(${IMGUI_BACKEND_NAME} PUBLIC arche::gui-core PRIVATE IMGUI)

set_target_properties(${IMGUI_BACKEND_NAME} PROPERTIES FOLDER "GUI/Backends")

add_library(arche::gui-backend-imgui ALIAS ${IMGUI_BACKEND_NAME})