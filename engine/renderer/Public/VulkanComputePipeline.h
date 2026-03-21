#pragma once
#ifdef ARCHE_BACKEND_VULKAN

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <cstdint>
#include <vector>

namespace Arche {
    namespace Render {

        /**
         * @brief Encapsulates a Vulkan compute pipeline with its layout and descriptor set layout.
         *
         * Created from pre-compiled SPIR-V bytecode. The caller is responsible for
         * binding descriptor sets before calling dispatch().
         */
        class VulkanComputePipeline {
          public:
            VulkanComputePipeline(VkDevice device, VmaAllocator allocator);
            ~VulkanComputePipeline();

            /**
             * @brief Create the pipeline from SPIR-V bytecode.
             *
             * @param spirv            Compiled SPIR-V words
             * @param bindings         Descriptor set layout bindings
             * @param pushConstantSize Byte size of push constant block (0 if unused)
             */
            void create(const std::vector<uint32_t> &spirv,
                        const std::vector<VkDescriptorSetLayoutBinding> &bindings,
                        uint32_t pushConstantSize = 0);

            /** @brief Destroy all Vulkan objects created by create(). */
            void destroy();

            /**
             * @brief Record a dispatch command into cmd.
             *
             * Descriptor sets and push constants must be bound by the caller before
             * this call.
             */
            void dispatch(VkCommandBuffer cmd,
                          uint32_t groupsX,
                          uint32_t groupsY = 1,
                          uint32_t groupsZ = 1);

            VkPipeline            pipeline()          const { return m_pipeline; }
            VkPipelineLayout      layout()            const { return m_layout; }
            VkDescriptorSetLayout descriptorSetLayout() const { return m_setLayout; }

          private:
            VkDevice              m_device;
            VmaAllocator          m_allocator;
            VkPipeline            m_pipeline{VK_NULL_HANDLE};
            VkPipelineLayout      m_layout{VK_NULL_HANDLE};
            VkDescriptorSetLayout m_setLayout{VK_NULL_HANDLE};
        };

    } // namespace Render
} // namespace Arche

#endif // ARCHE_BACKEND_VULKAN
