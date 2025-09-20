set(ARCHE_GUI_VIEWER "arche-gui-viewer")

add_executable(${ARCHE_GUI_VIEWER}
    main.cpp
)

target_link_libraries(${ARCHE_GUI_VIEWER} PRIVATE
    arche::gui-bundle
    arche::engine
)

target_include_directories(${ARCHE_GUI_VIEWER} PRIVATE
    ${CMAKE_SOURCE_DIR}/gui/core/Public
    ${CMAKE_SOURCE_DIR}/gui/backend/imgui/Public
    ${CMAKE_SOURCE_DIR}/gui/modules)