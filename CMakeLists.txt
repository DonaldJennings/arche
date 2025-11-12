# Minimum CMake version and project name
cmake_minimum_required(VERSION 3.15)
project(ArcheEngine)
project(ArcheEngine LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("Building BINARY_DIR: ${CMAKE_BINARY_DIR}")

# Ensure executable, static and shared library outputs go to ${CMAKE_BINARY_DIR}/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Organize targets in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

option(BUILD_TESTING "Build the testing tree." ON)

add_subdirectory(tools/glad)
add_subdirectory(tools/GLFW)
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools/glm)


find_package(OpenGL REQUIRED)

# Include the engine backend
add_subdirectory(engine)


set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools/imgui)
file(GLOB IMGUI_SOURCES
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(arche-imgui STATIC ${IMGUI_SOURCES})
target_include_directories(arche-imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(arche-imgui PUBLIC glfw OpenGL::GL)
add_library(arche::imgui ALIAS arche-imgui)

add_subdirectory(app)


file(GLOB_RECURSE ALL_SOURCE_FILES "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.h")

# Testing and third-party dependencies
if (BUILD_TESTING)
    message("Building tests...")
    add_subdirectory(tools/catch2)

    set_target_properties(Catch2 Catch2WithMain PROPERTIES FOLDER "Tools")
    add_subdirectory(tests)
endif()

# Group config files in a "Config" folder in Solution Explorer
set(CONFIG_FILES
    ${CMAKE_SOURCE_DIR}/.clang-tidy
)

# Create a dummy target to show config files in Solution Explorer
add_custom_target(ConfigFiles SOURCES ${CONFIG_FILES})

# Place them in a virtual folder called "Config"
source_group("Config" FILES ${CONFIG_FILES})

set_target_properties(ConfigFiles PROPERTIES FOLDER "ProjectConfiguration")

add_custom_target(clang_format 
    COMMAND clang-format -i ${ALL_SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running clang-format on source files"
)

set_target_properties(clang_format PROPERTIES FOLDER "Tools")