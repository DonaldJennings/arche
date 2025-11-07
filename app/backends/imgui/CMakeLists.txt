set(IMGUI_BACKEND_NAME "arche-gui-backend-imgui")

# Glob source files
file(GLOB_RECURSE IMGUI_BACKEND_SOURCES
    Private/*.cpp
    Public/*.h
)

add_library(${IMGUI_BACKEND_NAME} STATIC
    ${IMGUI_BACKEND_SOURCES}
)

# Expose includes from Public and also make Private and tools/imgui visible so local headers like stb_image.h can be found
target_include_directories(${IMGUI_BACKEND_NAME} PUBLIC
    Public
    ${CMAKE_SOURCE_DIR}/app/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/Private
    ${CMAKE_SOURCE_DIR}/tools/imgui
    ${stb_SOURCE_DIR} # provide stb headers
)


# Link dependencies
target_link_libraries(${IMGUI_BACKEND_NAME} PUBLIC arche::gui-core arche::imgui)

# Link GLEW if available (ImageManager uses <GL/glew.h>)
find_package(GLEW)
if (GLEW_FOUND)
    target_include_directories(${IMGUI_BACKEND_NAME} PRIVATE ${GLEW_INCLUDE_DIRS})
    target_link_libraries(${IMGUI_BACKEND_NAME} PRIVATE ${GLEW_LIBRARIES})
endif()

set_target_properties(${IMGUI_BACKEND_NAME} PROPERTIES FOLDER "GUI/Backends")

add_library(arche::gui-backend-imgui ALIAS ${IMGUI_BACKEND_NAME})

# after add_library(${IMGUI_BACKEND_NAME} ...)
set(FONT_SRC "${CMAKE_SOURCE_DIR}/assets/fonts/Roboto-Regular.ttf")
add_custom_command(TARGET ${IMGUI_BACKEND_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${IMGUI_BACKEND_NAME}>/assets/fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FONT_SRC}" "$<TARGET_FILE_DIR:${IMGUI_BACKEND_NAME}>/assets/fonts/Roboto-Regular.ttf"
)