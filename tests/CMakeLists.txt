cmake_minimum_required(VERSION 3.15)

set(TESTS_TARGET_NAME "arche-engine-tests")
file(GLOB_RECURSE TEST_SOURCES ./*.cpp)

add_executable(${TESTS_TARGET_NAME}
    ${TEST_SOURCES}
)

target_include_directories(${TESTS_TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/engine)

target_link_libraries(${TESTS_TARGET_NAME} PRIVATE
    arche::engine
    Catch2::Catch2 Catch2::Catch2WithMain
)

set_target_properties(${TESTS_TARGET_NAME} PROPERTIES FOLDER "Tests")

