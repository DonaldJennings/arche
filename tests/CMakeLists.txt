cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE TEST_SOURCES ./*.cpp)

add_executable(ArcheEngineBackendUnitTests
    ${TEST_SOURCES}
)

target_include_directories(ArcheEngineBackendUnitTests PUBLIC ${CMAKE_SOURCE_DIR}/engine)

target_link_libraries(ArcheEngineBackendUnitTests PRIVATE
    EngineCore EngineAPI EngineScene EngineMath
    Catch2::Catch2 Catch2::Catch2WithMain
)

set_target_properties(ArcheEngineBackendUnitTests PROPERTIES FOLDER "Tests")

