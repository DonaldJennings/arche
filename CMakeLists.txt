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

set(ARCHE_RENDERER_BACKEND "opengl" CACHE STRING "Renderer backend to use (opengl|vulkan)")
set_property(CACHE ARCHE_RENDERER_BACKEND PROPERTY STRINGS opengl vulkan)
message(STATUS "Renderer backend: ${ARCHE_RENDERER_BACKEND}")

add_subdirectory(tools/glad)
add_subdirectory(tools/GLFW)
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools/glm)

# VulkanMemoryAllocator — place the VMA headers in tools/vma/include/
set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools/vma)

find_package(OpenGL REQUIRED)

if(ARCHE_RENDERER_BACKEND STREQUAL "vulkan")
    find_package(Vulkan REQUIRED)
    message(STATUS "Vulkan SDK found: ${Vulkan_INCLUDE_DIRS}")
endif()

set(ENGINE_ASSET_DIR "${CMAKE_SOURCE_DIR}/engine/assets")
add_compile_definitions(ENGINE_ASSET_DIR="${ENGINE_ASSET_DIR}")


# Include the engine backend
add_subdirectory(engine)


set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools/imgui)

# Core ImGui sources (always included)
file(GLOB IMGUI_CORE_SOURCES ${IMGUI_DIR}/*.cpp)

if(ARCHE_RENDERER_BACKEND STREQUAL "vulkan")
    set(IMGUI_BACKEND_SOURCES
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    )
else()
    set(IMGUI_BACKEND_SOURCES
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
endif()

add_library(arche-imgui STATIC ${IMGUI_CORE_SOURCES} ${IMGUI_BACKEND_SOURCES})
target_include_directories(arche-imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

if(ARCHE_RENDERER_BACKEND STREQUAL "vulkan")
    target_compile_definitions(arche-imgui PUBLIC ARCHE_BACKEND_VULKAN)
    target_link_libraries(arche-imgui PUBLIC glfw Vulkan::Vulkan)
    # VMA headers needed when Vulkan backend is active
    if(EXISTS "${VMA_DIR}/include/vk_mem_alloc.h")
        target_include_directories(arche-imgui PUBLIC ${VMA_DIR}/include)
    endif()
else()
    target_link_libraries(arche-imgui PUBLIC glfw OpenGL::GL)
endif()

add_library(arche::imgui ALIAS arche-imgui)

set_target_properties(arche-imgui PROPERTIES FOLDER "Tools")

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
