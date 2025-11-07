set(CORE_TARGET_NAME "arche-engine-core")

file(GLOB_RECURSE CORE_SOURCES "Private/*.cpp")
file(GLOB_RECURSE CORE_HEADERS "Public/*.h")

add_library(${CORE_TARGET_NAME} STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# expose only the Public folder so includes do NOT need the Public/ prefix
target_include_directories(${CORE_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Public
													  ${PROJECT_SOURCE_DIR}/engine/)

target_link_libraries(${CORE_TARGET_NAME} PUBLIC arche::engine-api arche::engine-renderer)
set_target_properties(${CORE_TARGET_NAME} PROPERTIES FOLDER "Engine/Core")

add_library(arche::engine-core ALIAS ${CORE_TARGET_NAME})