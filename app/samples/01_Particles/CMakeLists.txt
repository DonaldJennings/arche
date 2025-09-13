cmake_minimum_required(VERSION 3.15)

project(ParticlesSample LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(ParticlesSample
    ParticlesDemo.cpp
    ParticlesDemo.h
)

# Link against engine libraries
target_link_libraries(ParticlesSample
    PRIVATE
        EngineCore
        EngineMath
        EngineScene
        EngineAPI
)

# Add include directories for engine headers
target_include_directories(ParticlesSample
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine
)

# Optionally, set output directory (inherits from top-level if set)
# set_target_properties(ParticlesSample PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
# )

# Organize in IDE
set_target_properties(ParticlesSample PROPERTIES FOLDER "App/Samples/01_Particles")