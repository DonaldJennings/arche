set(SYSTEM_TARGET_NAME "arche-engine-renderer")

file(GLOB_RECURSE ARCHE_SYSTEM_SOURCES "Private/*.cpp")
file(GLOB_RECURSE ARCHE_SYSTEM_HEADERS "Public/*.h")

add_library(${SYSTEM_TARGET_NAME} STATIC ${ARCHE_SYSTEM_SOURCES} ${ARCHE_SYSTEM_HEADERS})

target_include_directories(${SYSTEM_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Public)

target_link_libraries(${SYSTEM_TARGET_NAME} PUBLIC arche::engine-math arche::engine-core arche::engine-scene PUBLIC arche::glad glfw OpenGL::GL)

set_target_properties(${SYSTEM_TARGET_NAME} PROPERTIES FOLDER "Engine/Renderer")

add_library(arche::engine-renderer ALIAS ${SYSTEM_TARGET_NAME})