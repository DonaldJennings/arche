set(CORE_TARGET_NAME "arche-engine-core")

file(GLOB_RECURSE CORE_SOURCES "Private/*.cpp")
file(GLOB_RECURSE CORE_HEADERS "Public/*.h")

add_library(${CORE_TARGET_NAME} STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(${CORE_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${CORE_TARGET_NAME} PROPERTIES FOLDER "Engine/Core")

add_library(arche::engine-core ALIAS ${CORE_TARGET_NAME})