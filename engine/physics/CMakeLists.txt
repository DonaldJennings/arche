set(SYSTEM_TARGET_NAME "arche-engine-physics")

file(GLOB_RECURSE ARCHE_SYSTEM_SOURCES "Private/*.cpp")
file(GLOB_RECURSE ARCHE_SYSTEM_HEADERS "Public/*.h")

find_package(OpenMP)

add_library(${SYSTEM_TARGET_NAME} STATIC ${ARCHE_SYSTEM_SOURCES} ${ARCHE_SYSTEM_HEADERS})

message("GLM DIR: ${GLM_DIR}")
target_include_directories(${SYSTEM_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Public ${GLM_DIR})

target_link_libraries(${SYSTEM_TARGET_NAME} PUBLIC arche::engine-core arche::engine-scene)

if(OpenMP_CXX_FOUND)
    target_link_libraries(${SYSTEM_TARGET_NAME} PUBLIC OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found — physics integration will run in parallel")
else()
    message(STATUS "OpenMP not found — physics integration will run single-threaded")
endif()

set_target_properties(${SYSTEM_TARGET_NAME} PROPERTIES FOLDER "Engine/Physics")

add_library(arche::engine-physics ALIAS ${SYSTEM_TARGET_NAME})