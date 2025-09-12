cmake_minimum_required(VERSION 3.15)

enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES Test*.cpp)

add_executable(ArcheEngineBackendUnitTests
    ${TEST_SOURCES}
)

target_link_libraries(ArcheEngineBackendUnitTests
    GTest::gtest
    GTest::gtest_main
)

target_compile_features(ArcheEngineBackendUnitTests PRIVATE cxx_std_23)

add_test(NAME ArcheEngineBackendUnitTests COMMAND ArcheEngineBackendUnitTests)