#include "VulkanComputePipeline.h"
#ifdef ARCHE_BACKEND_VULKAN

#include <stdexcept>

namespace Arche {
    namespace Render {

        VulkanComputePipeline::VulkanComputePipeline(VkDevice device, VmaAllocator allocator)
            : m_device(device), m_allocator(allocator) {}

        VulkanComputePipeline::~VulkanComputePipeline() {
            destroy();
        }

        void VulkanComputePipeline::create(
            const std::vector<uint32_t> &spirv,
            const std::vector<VkDescriptorSetLayoutBinding> &bindings,
            uint32_t pushConstantSize)
        {
            // --- Descriptor set layout ---
            VkDescriptorSetLayoutCreateInfo dslCI{};
            dslCI.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            dslCI.bindingCount = static_cast<uint32_t>(bindings.size());
            dslCI.pBindings    = bindings.empty() ? nullptr : bindings.data();

            if (vkCreateDescriptorSetLayout(m_device, &dslCI, nullptr, &m_setLayout) != VK_SUCCESS)
                throw std::runtime_error("VulkanComputePipeline: failed to create descriptor set layout");

            // --- Pipeline layout ---
            VkPushConstantRange pcRange{};
            pcRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            pcRange.offset     = 0;
            pcRange.size       = pushConstantSize;

            VkPipelineLayoutCreateInfo layoutCI{};
            layoutCI.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layoutCI.setLayoutCount         = 1;
            layoutCI.pSetLayouts            = &m_setLayout;
            layoutCI.pushConstantRangeCount = pushConstantSize > 0 ? 1 : 0;
            layoutCI.pPushConstantRanges    = pushConstantSize > 0 ? &pcRange : nullptr;

            if (vkCreatePipelineLayout(m_device, &layoutCI, nullptr, &m_layout) != VK_SUCCESS)
                throw std::runtime_error("VulkanComputePipeline: failed to create pipeline layout");

            // --- Shader module ---
            VkShaderModuleCreateInfo smCI{};
            smCI.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            smCI.codeSize = spirv.size() * sizeof(uint32_t);
            smCI.pCode    = spirv.data();

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(m_device, &smCI, nullptr, &shaderModule) != VK_SUCCESS)
                throw std::runtime_error("VulkanComputePipeline: failed to create shader module");

            // --- Compute pipeline ---
            VkComputePipelineCreateInfo pipeCI{};
            pipeCI.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipeCI.layout = m_layout;
            pipeCI.stage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            pipeCI.stage.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
            pipeCI.stage.module = shaderModule;
            pipeCI.stage.pName  = "main";

            VkResult result = vkCreateComputePipelines(
                m_device, VK_NULL_HANDLE, 1, &pipeCI, nullptr, &m_pipeline);

            vkDestroyShaderModule(m_device, shaderModule, nullptr);

            if (result != VK_SUCCESS)
                throw std::runtime_error("VulkanComputePipeline: failed to create compute pipeline");
        }

        void VulkanComputePipeline::destroy() {
            if (m_pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(m_device, m_pipeline, nullptr);
                m_pipeline = VK_NULL_HANDLE;
            }
            if (m_layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(m_device, m_layout, nullptr);
                m_layout = VK_NULL_HANDLE;
            }
            if (m_setLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(m_device, m_setLayout, nullptr);
                m_setLayout = VK_NULL_HANDLE;
            }
        }

        void VulkanComputePipeline::dispatch(VkCommandBuffer cmd,
                                             uint32_t groupsX,
                                             uint32_t groupsY,
                                             uint32_t groupsZ)
        {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
            vkCmdDispatch(cmd, groupsX, groupsY, groupsZ);
        }

    } // namespace Render
} // namespace Arche

#endif // ARCHE_BACKEND_VULKAN
