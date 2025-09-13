# Minimum CMake version and project name
cmake_minimum_required(VERSION 3.15)
project(ArcheEngine)
project(ArcheEngine LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Organize targets in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

option(BUILD_TESTING "Build the testing tree." ON)

# Add engine and app subdirectories
# Add subdomains as subdirectories
add_subdirectory(engine/core)
add_subdirectory(engine/math)
add_subdirectory(engine/api)
add_subdirectory(engine/scene)

add_subdirectory(app/samples/01_Particles)

# Testing and third-party dependencies
if (BUILD_TESTING)
    message("Building tests...")
    add_subdirectory(tools/catch2)
    add_subdirectory(tests)
endif()

# Set output directories for all targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
