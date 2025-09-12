cmake_minimum_required(VERSION 3.15)

project(ArcheBackendEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdomains as subdirectories
add_subdirectory(core)
add_subdirectory(math)
add_subdirectory(api)
add_subdirectory(scene)