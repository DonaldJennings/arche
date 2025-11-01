set(MATH_TARGET_NAME "arche-engine-math")

file(GLOB_RECURSE MATH_SOURCES "Private/*.cpp")
file(GLOB_RECURSE MATH_HEADERS "Public/*.h")

add_library(${MATH_TARGET_NAME} STATIC ${MATH_SOURCES} ${MATH_HEADERS})

target_include_directories(${MATH_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Public)

set_target_properties(${MATH_TARGET_NAME} PROPERTIES FOLDER "Engine/Math")

add_library(arche::engine-math ALIAS ${MATH_TARGET_NAME})