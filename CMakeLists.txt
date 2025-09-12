# Minimum CMake version and project name
cmake_minimum_required(VERSION 3.15)
project(ArcheEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories for engine, app, and tests
add_subdirectory(engine)
add_subdirectory(app/samples/01_Particles)
add_subdirectory(tests)

# Enable testing
enable_testing()
include(CTest)