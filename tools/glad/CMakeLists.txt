cmake_minimum_required(VERSION 3.14)
project(arche-glad VERSION 1.0.0 LANGUAGES C)

file(GLOB_RECURSE GLAD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# DEBUG: print what was found
message(STATUS "Tools/glad: GLAD_SOURCES='${GLAD_SOURCES}'")

# Robust check for empty list
list(LENGTH GLAD_SOURCES GLAD_SOURCES_COUNT)
message(STATUS "Tools/glad: found ${GLAD_SOURCES_COUNT} source(s)")
if(GLAD_SOURCES_COUNT EQUAL 0)
    message(FATAL_ERROR "Could not find any glad .c sources in ${CMAKE_CURRENT_SOURCE_DIR}.")
endif()

add_library(arche-glad STATIC ${GLAD_SOURCES})
target_include_directories(arche-glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(arche-glad PUBLIC c_std_99)
set_target_properties(arche-glad PROPERTIES FOLDER "Tools")
add_library(arche::glad ALIAS arche-glad)