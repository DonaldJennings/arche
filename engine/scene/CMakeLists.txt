set(SCENE_TARGET_NAME "arche-engine-scene")

file(GLOB_RECURSE SCENE_SOURCES "Private/*.cpp")
file(GLOB_RECURSE SCENE_HEADERS "Public/*.h")

add_library(${SCENE_TARGET_NAME} STATIC ${SCENE_SOURCES} ${SCENE_HEADERS})

target_include_directories(${SCENE_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${SCENE_TARGET_NAME} PUBLIC arche::engine-math arche::engine-core)

set_target_properties(${SCENE_TARGET_NAME} PROPERTIES FOLDER "Engine/Scene")

add_library(arche::engine-scene ALIAS ${SCENE_TARGET_NAME})