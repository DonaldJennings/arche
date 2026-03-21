// VMA_IMPLEMENTATION must be defined in exactly one .cpp file.
#define VMA_IMPLEMENTATION
#include "VulkanResourceManager.h"

#ifdef ARCHE_BACKEND_VULKAN

#include <stdexcept>
#include <cstring>

namespace Arche {
    namespace Render {

        VulkanResourceManager::VulkanResourceManager(VkDevice device, VmaAllocator allocator)
            : m_device(device), m_allocator(allocator) {}

        VulkanResourceManager::~VulkanResourceManager() {}

        // -----------------------------------------------------------------------
        // Internal helper: create a raw buffer + allocation
        // -----------------------------------------------------------------------
        static GpuBuffer createBuffer(VmaAllocator allocator,
                                      VkDeviceSize size,
                                      VkBufferUsageFlags usage,
                                      VmaMemoryUsage memUsage)
        {
            VkBufferCreateInfo bufCI{};
            bufCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufCI.size  = size;
            bufCI.usage = usage;
            bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocCI{};
            allocCI.usage = memUsage;

            GpuBuffer buf;
            buf.size = size;
            if (vmaCreateBuffer(allocator, &bufCI, &allocCI,
                                &buf.buffer, &buf.allocation, nullptr) != VK_SUCCESS)
                throw std::runtime_error("VulkanResourceManager: vmaCreateBuffer failed");

            return buf;
        }

        // -----------------------------------------------------------------------
        // Internal helper: one-shot command buffer submit
        // -----------------------------------------------------------------------
        static VkCommandBuffer beginOneShot(VkDevice device, VkCommandPool cmdPool)
        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool        = cmdPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer cmd;
            vkAllocateCommandBuffers(device, &allocInfo, &cmd);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &beginInfo);

            return cmd;
        }

        static void endOneShot(VkDevice device, VkCommandPool cmdPool,
                               VkQueue queue, VkCommandBuffer cmd)
        {
            vkEndCommandBuffer(cmd);

            VkSubmitInfo submitInfo{};
            submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = &cmd;

            vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(queue);
            vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
        }

        // -----------------------------------------------------------------------
        // Public API
        // -----------------------------------------------------------------------

        GpuBuffer VulkanResourceManager::createDeviceBuffer(VkDeviceSize size,
                                                             VkBufferUsageFlags usage,
                                                             const void *data,
                                                             VkCommandPool cmdPool,
                                                             VkQueue queue)
        {
            // Staging buffer (host visible)
            GpuBuffer staging = createBuffer(m_allocator, size,
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                             VMA_MEMORY_USAGE_CPU_ONLY);

            if (data) {
                void *mapped = nullptr;
                vmaMapMemory(m_allocator, staging.allocation, &mapped);
                std::memcpy(mapped, data, static_cast<size_t>(size));
                vmaUnmapMemory(m_allocator, staging.allocation);
            }

            // Device-local buffer
            GpuBuffer deviceBuf = createBuffer(m_allocator, size,
                                               usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                               VMA_MEMORY_USAGE_GPU_ONLY);

            // Copy staging → device
            VkCommandBuffer cmd = beginOneShot(m_device, cmdPool);
            VkBufferCopy region{0, 0, size};
            vkCmdCopyBuffer(cmd, staging.buffer, deviceBuf.buffer, 1, &region);
            endOneShot(m_device, cmdPool, queue, cmd);

            // Free staging
            vmaDestroyBuffer(m_allocator, staging.buffer, staging.allocation);

            return deviceBuf;
        }

        GpuBuffer VulkanResourceManager::createHostBuffer(VkDeviceSize size,
                                                          VkBufferUsageFlags usage)
        {
            return createBuffer(m_allocator, size, usage, VMA_MEMORY_USAGE_CPU_TO_GPU);
        }

        void VulkanResourceManager::destroyBuffer(GpuBuffer &buf)
        {
            if (buf.buffer != VK_NULL_HANDLE) {
                vmaDestroyBuffer(m_allocator, buf.buffer, buf.allocation);
                buf.buffer     = VK_NULL_HANDLE;
                buf.allocation = VK_NULL_HANDLE;
                buf.size       = 0;
            }
        }

    } // namespace Render
} // namespace Arche

#endif // ARCHE_BACKEND_VULKAN
