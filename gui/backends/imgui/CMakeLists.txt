set(IMGUI-BACKEND_NAME "arche-gui-backend-imgui")

add_library(${IMGUI-BACKEND_NAME} STATIC
    Private/ImGuiBackend.cpp
    Public/ImGuiBackend.h
)

target_include_directories(${IMGUI-BACKEND_NAME} PUBLIC
    Public
)

# Link dependencies
target_link_libraries(${IMGUI-BACKEND_NAME} PUBLIC arche::gui-core imgui)

add_library(arche::gui-backend-imgui ALIAS ${IMGUI-BACKEND_NAME})