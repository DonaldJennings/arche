set(IMGUI_BACKEND_NAME "arche-gui-backend-imgui")

# Glob source files
file(GLOB_RECURSE IMGUI_BACKEND_SOURCES
    Private/*.cpp
    Public/*.h
)

add_library(${IMGUI_BACKEND_NAME} STATIC
    ${IMGUI_BACKEND_SOURCES}
)

target_include_directories(${IMGUI_BACKEND_NAME} PUBLIC
    Public
    ${CMAKE_SOURCE_DIR}/gui/imgui
)


# Link dependencies
target_link_libraries(${IMGUI_BACKEND_NAME} PUBLIC arche::gui-core PRIVATE IMGUI)

set_target_properties(${IMGUI_BACKEND_NAME} PROPERTIES FOLDER "GUI/Backends")

add_library(arche::gui-backend-imgui ALIAS ${IMGUI_BACKEND_NAME})