set(SYSTEM_TARGET_NAME "arche-engine-renderer")

file(GLOB_RECURSE ARCHE_SYSTEM_SOURCES "Private/*.cpp")
file(GLOB_RECURSE ARCHE_SYSTEM_HEADERS "Public/*.h")

# Exclude backend-specific sources that don't apply to the selected backend
if(ARCHE_RENDERER_BACKEND STREQUAL "vulkan")
    list(FILTER ARCHE_SYSTEM_SOURCES EXCLUDE REGEX ".*OpenGLBackend\\.cpp$")
else()
    list(FILTER ARCHE_SYSTEM_SOURCES EXCLUDE REGEX ".*(VulkanBackend|VulkanResourceManager|VulkanComputePipeline)\\.cpp$")
endif()

add_library(${SYSTEM_TARGET_NAME} STATIC ${ARCHE_SYSTEM_SOURCES} ${ARCHE_SYSTEM_HEADERS})

message("GLM DIR: ${GLM_DIR}")
target_include_directories(${SYSTEM_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Public ${GLM_DIR})

# Private include so internal .cpp files can include headers in Private/
target_include_directories(${SYSTEM_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private)

if(ARCHE_RENDERER_BACKEND STREQUAL "vulkan")
    target_compile_definitions(${SYSTEM_TARGET_NAME} PUBLIC ARCHE_BACKEND_VULKAN)
    target_link_libraries(${SYSTEM_TARGET_NAME} PUBLIC
        arche::engine-core arche::engine-scene arche::engine-physics
        Vulkan::Vulkan glfw
    )
    # VMA headers (tools/vma/include must contain vk_mem_alloc.h)
    if(EXISTS "${VMA_DIR}/include/vk_mem_alloc.h")
        target_include_directories(${SYSTEM_TARGET_NAME} PUBLIC ${VMA_DIR}/include)
        message(STATUS "VMA found at ${VMA_DIR}/include")
    else()
        message(WARNING "VMA not found at ${VMA_DIR}/include — add vk_mem_alloc.h to tools/vma/include/")
    endif()
else()
    target_link_libraries(${SYSTEM_TARGET_NAME} PUBLIC
        arche::engine-core arche::engine-scene arche::engine-physics
        arche::glad glfw OpenGL::GL
    )
endif()

set_target_properties(${SYSTEM_TARGET_NAME} PROPERTIES FOLDER "Engine/Renderer")

add_library(arche::engine-renderer ALIAS ${SYSTEM_TARGET_NAME})
