set(ARCHE_GUI_VIEWER "arche-gui-viewer")

add_executable(${ARCHE_GUI_VIEWER}
    main.cpp
)

target_link_libraries(${ARCHE_GUI_VIEWER} PRIVATE
    arche::gui-bundle
    arche::engine
)

target_include_directories(${ARCHE_GUI_VIEWER} PRIVATE
    ${CMAKE_SOURCE_DIR}/app/core/Public
    ${CMAKE_SOURCE_DIR}/app/backend/imgui/Public
    ${CMAKE_SOURCE_DIR}/app/modules
)


set_target_properties(${ARCHE_GUI_VIEWER} PROPERTIES FOLDER "GUI/Application")

# Run from the executable directory in Visual Studio
set_target_properties(${ARCHE_GUI_VIEWER} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${ARCHE_GUI_VIEWER}>"
)

# Copy assets into the target binary directory so runtime can open "assets/..."
set(ASSETS_SRC_DIR "${CMAKE_SOURCE_DIR}/assets")
add_custom_command(TARGET ${ARCHE_GUI_VIEWER} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${ARCHE_GUI_VIEWER}>/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${ARCHE_GUI_VIEWER}>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_SRC_DIR}" "$<TARGET_FILE_DIR:${ARCHE_GUI_VIEWER}>/assets"
    COMMENT "Copying assets to $<TARGET_FILE_DIR:${ARCHE_GUI_VIEWER}>/assets"
)
