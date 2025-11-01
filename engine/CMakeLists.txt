cmake_minimum_required(VERSION 3.15)

set(ARCHE_ENGINE_NAME "arche-engine")
set(ARCHE_ENGINE_BUNDLE_NAME "arche-engine-bundle")

add_subdirectory(core)
add_subdirectory(math)
add_subdirectory(api)
add_subdirectory(scene)
add_subdirectory(renderer)

add_library(arche-engine-bundle INTERFACE)

target_link_libraries(arche-engine-bundle INTERFACE
    arche::engine-core
    arche::engine-math
    arche::engine-api
    arche::engine-scene
    arche::engine-renderer
)

add_library(arche::engine ALIAS arche-engine-bundle)