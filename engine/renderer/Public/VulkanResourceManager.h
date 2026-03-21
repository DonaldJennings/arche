#pragma once
#ifdef ARCHE_BACKEND_VULKAN

#include <vulkan/vulkan.h>

// Include VMA without implementation — only VulkanResourceManager.cpp defines VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <cstdint>

namespace Arche {
    namespace Render {

        /**
         * @brief GPU buffer wrapper holding a VkBuffer + VmaAllocation pair.
         */
        struct GpuBuffer {
            VkBuffer      buffer{VK_NULL_HANDLE};
            VmaAllocation allocation{VK_NULL_HANDLE};
            VkDeviceSize  size{0};
        };

        /**
         * @brief Manages GPU buffer allocation using VulkanMemoryAllocator.
         *
         * Provides helpers to create device-local buffers (via staging upload)
         * and persistently mapped host-visible buffers. VMA_IMPLEMENTATION is
         * defined only in VulkanResourceManager.cpp.
         */
        class VulkanResourceManager {
          public:
            /**
             * @brief Construct with an externally created VmaAllocator.
             *
             * The caller (VulkanBackend) owns the allocator lifetime; this manager
             * does NOT destroy it.
             */
            VulkanResourceManager(VkDevice device, VmaAllocator allocator);
            ~VulkanResourceManager();

            /**
             * @brief Create a device-local buffer, uploading data via a staging buffer.
             *
             * @param size      Byte size of the data
             * @param usage     Buffer usage flags (e.g. VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
             * @param data      Pointer to host data to upload (may be nullptr)
             * @param cmdPool   Command pool used for the one-shot transfer command
             * @param queue     Queue on which to submit the transfer
             */
            GpuBuffer createDeviceBuffer(VkDeviceSize size,
                                         VkBufferUsageFlags usage,
                                         const void *data,
                                         VkCommandPool cmdPool,
                                         VkQueue queue);

            /**
             * @brief Create a persistently mapped host-visible buffer.
             *
             * Suitable for uniform buffers or staging buffers that are written
             * every frame.
             *
             * @param size  Byte size
             * @param usage Buffer usage flags
             */
            GpuBuffer createHostBuffer(VkDeviceSize size,
                                       VkBufferUsageFlags usage);

            /** @brief Destroy a buffer and free its allocation. */
            void destroyBuffer(GpuBuffer &buf);

            VmaAllocator allocator() const { return m_allocator; }

          private:
            VkDevice     m_device;
            VmaAllocator m_allocator;
        };

    } // namespace Render
} // namespace Arche

#endif // ARCHE_BACKEND_VULKAN
