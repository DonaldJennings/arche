set(API_TARGET_NAME "EngineApi")

file(GLOB_RECURSE API_SOURCES "*.cpp")
file(GLOB_RECURSE API_HEADERS "*.h")

add_library(${API_TARGET_NAME} STATIC ${API_SOURCES} ${API_HEADERS})

target_include_directories(${API_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${API_TARGET_NAME} PROPERTIES FOLDER "Engine/Api")