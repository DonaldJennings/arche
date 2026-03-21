#include "VulkanBackend.h"
#ifdef ARCHE_BACKEND_VULKAN

#include "Mesh.h"
#include "GeometryVertSPIRV.h"
#include "GeometryFragSPIRV.h"
#include "PhysicsShaderSPIRV.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <limits>
#include <set>
#include <stdexcept>
#include <vector>

namespace Arche {
    namespace Render {

        // ============================================================
        // Static constants
        // ============================================================

        const std::vector<const char *> VulkanBackend::k_validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        const std::vector<const char *> VulkanBackend::k_deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        // ============================================================
        // Debug callback
        // ============================================================

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
            VkDebugUtilsMessageTypeFlagsEXT             /*type*/,
            const VkDebugUtilsMessengerCallbackDataEXT *data,
            void *)
        {
            if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                fprintf(stderr, "[Vulkan] %s\n", data->pMessage);
            return VK_FALSE;
        }

        static VkResult CreateDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT *pCI,
            const VkAllocationCallbacks *pAlloc,
            VkDebugUtilsMessengerEXT *pMsg)
        {
            auto fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            return fn ? fn(instance, pCI, pAlloc, pMsg) : VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        static void DestroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT messenger,
            const VkAllocationCallbacks *pAlloc)
        {
            auto fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (fn) fn(instance, messenger, pAlloc);
        }

        // ============================================================
        // Constructor
        // ============================================================

        VulkanBackend::VulkanBackend(GLFWwindow *window) : m_window(window) {}

        // ============================================================
        // Lifecycle
        // ============================================================

        void VulkanBackend::initialise() {
            createInstance();
            if (k_enableValidation) setupDebugMessenger();
            createSurface();
            pickPhysicalDevice();
            createLogicalDevice();
            createAllocator();           // F-10: VMA
            createCommandPool();
            recreateSwapchain();         // F-07: swapchain + render passes + framebuffers
            createSyncObjects();
            createCommandBuffers();
            createDescriptorPool();
            createDescriptorSetLayout(); // F-08: geometry UBO layout
            createUBOs();
            createDescriptorSets();
            // F-08: graphics pipeline (gracefully skipped if SPIR-V stubs fail validation)
            try {
                createGeometryPipeline();
            } catch (const std::exception &e) {
                fprintf(stderr, "[VulkanBackend] Geometry pipeline not created: %s\n", e.what());
                fprintf(stderr, "[VulkanBackend] Replace embedded SPIR-V stubs with output of "
                        "glslangValidator to enable geometry rendering.\n");
                m_geometryPipelineReady = false;
            }
            // F-11: compute pipeline (gracefully skipped if SPIR-V stubs fail validation)
            try {
                createComputePipeline();
            } catch (const std::exception &e) {
                fprintf(stderr, "[VulkanBackend] Compute pipeline not created: %s\n", e.what());
            }

            // F-09: populate ImGui context struct
            m_imguiContext.instance             = m_instance;
            m_imguiContext.physicalDevice       = m_physicalDevice;
            m_imguiContext.device               = m_device;
            m_imguiContext.graphicsQueue        = m_graphicsQueue;
            m_imguiContext.graphicsQueueFamily  = m_queueFamilies.graphicsFamily.value();
            m_imguiContext.imguiRenderPass      = m_imguiRenderPass;
            m_imguiContext.minImageCount        = 2;
            m_imguiContext.imageCount           = m_swapchainImageCount;
        }

        void VulkanBackend::shutdown() noexcept {
            if (m_device == VK_NULL_HANDLE) return;

            vkDeviceWaitIdle(m_device);

            // Physics SSBO
            if (m_physicsDescPool != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(m_device, m_physicsDescPool, nullptr);
                m_physicsDescPool = VK_NULL_HANDLE;
            }
            if (m_physicsDescSetLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(m_device, m_physicsDescSetLayout, nullptr);
                m_physicsDescSetLayout = VK_NULL_HANDLE;
            }
            if (m_resourceManager) {
                m_resourceManager->destroyBuffer(m_physicsBuffer);
                m_resourceManager->destroyBuffer(m_physicsReadbackBuffer);
            }

            // Compute pipeline
            if (m_computePipeline) m_computePipeline.reset();

            // Geometry pipeline
            if (m_geometryPipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(m_device, m_geometryPipeline, nullptr);
                m_geometryPipeline = VK_NULL_HANDLE;
            }
            if (m_geometryPipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(m_device, m_geometryPipelineLayout, nullptr);
                m_geometryPipelineLayout = VK_NULL_HANDLE;
            }
            if (m_descSetLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(m_device, m_descSetLayout, nullptr);
                m_descSetLayout = VK_NULL_HANDLE;
            }
            if (m_descriptorPool != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
                m_descriptorPool = VK_NULL_HANDLE;
            }
            for (int i = 0; i < k_maxFramesInFlight; ++i) {
                if (m_resourceManager) m_resourceManager->destroyBuffer(m_uboBuffers[i]);
            }

            // GPU meshes
            for (auto &[name, gm] : m_gpuMeshes) {
                if (m_resourceManager) {
                    m_resourceManager->destroyBuffer(gm.vertexBuffer);
                    m_resourceManager->destroyBuffer(gm.indexBuffer);
                }
            }
            m_gpuMeshes.clear();

            // Sync objects
            for (int i = 0; i < k_maxFramesInFlight; ++i) {
                if (m_renderFinishedSemaphores[i] != VK_NULL_HANDLE)
                    vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
                if (m_imageAvailableSemaphores[i] != VK_NULL_HANDLE)
                    vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
                if (m_inFlightFences[i] != VK_NULL_HANDLE)
                    vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
            }

            cleanupSwapchain();

            if (m_commandPool != VK_NULL_HANDLE) {
                vkDestroyCommandPool(m_device, m_commandPool, nullptr);
                m_commandPool = VK_NULL_HANDLE;
            }

            // VMA
            m_resourceManager.reset();
            if (m_allocator != VK_NULL_HANDLE) {
                vmaDestroyAllocator(m_allocator);
                m_allocator = VK_NULL_HANDLE;
            }

            vkDestroyDevice(m_device, nullptr);
            m_device = VK_NULL_HANDLE;

            if (m_debugMessenger != VK_NULL_HANDLE) {
                DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
                m_debugMessenger = VK_NULL_HANDLE;
            }
            if (m_surface != VK_NULL_HANDLE) {
                vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
                m_surface = VK_NULL_HANDLE;
            }
            if (m_instance != VK_NULL_HANDLE) {
                vkDestroyInstance(m_instance, nullptr);
                m_instance = VK_NULL_HANDLE;
            }
        }

        void VulkanBackend::resize(glm::ivec2 newSize) {
            m_backBufferSize     = newSize;
            m_framebufferResized = true;
        }

        // ============================================================
        // F-07: Frame loop
        // ============================================================

        void VulkanBackend::beginFrame() {
            // Wait for the in-flight fence of this frame slot
            vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame],
                            VK_TRUE, std::numeric_limits<uint64_t>::max());

            // Acquire a swapchain image
            VkResult result = vkAcquireNextImageKHR(
                m_device, m_swapchain,
                std::numeric_limits<uint64_t>::max(),
                m_imageAvailableSemaphores[m_currentFrame],
                VK_NULL_HANDLE,
                &m_imageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || m_framebufferResized) {
                m_framebufferResized = false;
                recreateSwapchain();
                m_frameStarted = false;
                return;
            } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                throw std::runtime_error("VulkanBackend: failed to acquire swapchain image");
            }

            vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

            // Reset and begin command buffer
            VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];
            vkResetCommandBuffer(cmd, 0);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
                throw std::runtime_error("VulkanBackend: failed to begin command buffer");

            // --- Geometry render pass ---
            VkRenderPassBeginInfo rpInfo{};
            rpInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpInfo.renderPass  = m_geometryRenderPass;
            rpInfo.framebuffer = m_geometryFramebuffers[m_imageIndex];
            rpInfo.renderArea.offset = {0, 0};
            rpInfo.renderArea.extent = m_swapchainExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color        = {{0.10f, 0.12f, 0.15f, 1.0f}};
            clearValues[1].depthStencil = {1.0f, 0};
            rpInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            rpInfo.pClearValues    = clearValues.data();

            vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

            // Set dynamic viewport + scissor
            VkViewport viewport{};
            viewport.x        = 0.0f;
            viewport.y        = 0.0f;
            viewport.width    = static_cast<float>(m_swapchainExtent.width);
            viewport.height   = static_cast<float>(m_swapchainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(cmd, 0, 1, &viewport);

            VkRect2D scissor{{0, 0}, m_swapchainExtent};
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            // Bind geometry pipeline and UBO descriptor set
            if (m_geometryPipelineReady) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_geometryPipeline);
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        m_geometryPipelineLayout, 0, 1,
                                        &m_descriptorSets[m_currentFrame],
                                        0, nullptr);
            }

            // Update UBO with current view/proj
            if (m_uboMapped[m_currentFrame]) {
                std::memcpy(m_uboMapped[m_currentFrame], &m_viewProj, sizeof(ViewProjUBO));
            }

            m_frameStarted = true;
        }

        void VulkanBackend::endFrame() {
            if (!m_frameStarted) return;
            m_frameStarted = false;

            VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];

            // End geometry render pass
            vkCmdEndRenderPass(cmd);

            // --- ImGui render pass (LOAD_OP_LOAD renders on top) ---
            VkRenderPassBeginInfo imguiRpInfo{};
            imguiRpInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            imguiRpInfo.renderPass  = m_imguiRenderPass;
            imguiRpInfo.framebuffer = m_imguiFramebuffers[m_imageIndex];
            imguiRpInfo.renderArea.offset = {0, 0};
            imguiRpInfo.renderArea.extent = m_swapchainExtent;
            // No clear — LOAD_OP_LOAD keeps geometry output
            imguiRpInfo.clearValueCount = 0;
            imguiRpInfo.pClearValues    = nullptr;

            vkCmdBeginRenderPass(cmd, &imguiRpInfo, VK_SUBPASS_CONTENTS_INLINE);
            if (m_imguiRenderCallback) {
                m_imguiRenderCallback(cmd);
            }
            vkCmdEndRenderPass(cmd);

            if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
                throw std::runtime_error("VulkanBackend: failed to record command buffer");

            // Submit
            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo submitInfo{};
            submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount   = 1;
            submitInfo.pWaitSemaphores      = &m_imageAvailableSemaphores[m_currentFrame];
            submitInfo.pWaitDstStageMask    = &waitStage;
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &cmd;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores    = &m_renderFinishedSemaphores[m_currentFrame];

            if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo,
                              m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
                throw std::runtime_error("VulkanBackend: failed to submit draw command buffer");

            // Present
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores    = &m_renderFinishedSemaphores[m_currentFrame];
            presentInfo.swapchainCount     = 1;
            presentInfo.pSwapchains        = &m_swapchain;
            presentInfo.pImageIndices      = &m_imageIndex;

            VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                recreateSwapchain();
            } else if (result != VK_SUCCESS) {
                throw std::runtime_error("VulkanBackend: failed to present swapchain image");
            }

            m_currentFrame = (m_currentFrame + 1) % k_maxFramesInFlight;

            // Throttled CPU readback counter
            ++m_readbackFrameCounter;
        }

        // ============================================================
        // F-08: View/projection + draw mesh
        // ============================================================

        void VulkanBackend::setViewProjection(const glm::mat4 &view, const glm::mat4 &proj) {
            m_viewProj.view = view;
            m_viewProj.proj = proj;
        }

        void VulkanBackend::drawMesh(const Mesh &mesh, const glm::mat4 &model) {
            if (!m_geometryPipelineReady || !m_frameStarted) return;

            // Lazy upload
            if (m_gpuMeshes.find(mesh.getName()) == m_gpuMeshes.end()) {
                uploadMesh(mesh);
            }

            auto it = m_gpuMeshes.find(mesh.getName());
            if (it == m_gpuMeshes.end()) return;

            const GpuMesh &gm  = it->second;
            VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];

            // Push model matrix
            vkCmdPushConstants(cmd, m_geometryPipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4),
                               glm::value_ptr(model));

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &gm.vertexBuffer.buffer, &offset);

            if (gm.indexCount > 0) {
                vkCmdBindIndexBuffer(cmd, gm.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(cmd, gm.indexCount, 1, 0, 0, 0);
            } else {
                vkCmdDraw(cmd, gm.vertexCount, 1, 0, 0);
            }
        }

        // ============================================================
        // F-06: Instance, debug, surface, physical/logical device
        // ============================================================

        void VulkanBackend::createInstance() {
            if (k_enableValidation && !checkValidationLayerSupport())
                throw std::runtime_error("Vulkan: requested validation layers not available");

            VkApplicationInfo appInfo{};
            appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName   = "Arche Engine";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName        = "Arche";
            appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion         = VK_API_VERSION_1_2;

            auto extensions = getRequiredExtensions();

            VkInstanceCreateInfo createInfo{};
            createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo        = &appInfo;
            createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debugCI{};
            if (k_enableValidation) {
                createInfo.enabledLayerCount   = static_cast<uint32_t>(k_validationLayers.size());
                createInfo.ppEnabledLayerNames = k_validationLayers.data();

                debugCI.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                debugCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                debugCI.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                debugCI.pfnUserCallback = debugCallback;
                createInfo.pNext = &debugCI;
            } else {
                createInfo.enabledLayerCount = 0;
            }

            if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create instance");
        }

        void VulkanBackend::setupDebugMessenger() {
            VkDebugUtilsMessengerCreateInfoEXT ci{};
            ci.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            ci.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            ci.pfnUserCallback = debugCallback;

            if (CreateDebugUtilsMessengerEXT(m_instance, &ci, nullptr, &m_debugMessenger)
                != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to set up debug messenger");
        }

        void VulkanBackend::createSurface() {
            if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create window surface");
        }

        void VulkanBackend::pickPhysicalDevice() {
            uint32_t count = 0;
            vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
            if (count == 0)
                throw std::runtime_error("Vulkan: no GPUs with Vulkan support found");

            std::vector<VkPhysicalDevice> devices(count);
            vkEnumeratePhysicalDevices(m_instance, &count, devices.data());

            for (const auto &d : devices) {
                VkPhysicalDeviceProperties p;
                vkGetPhysicalDeviceProperties(d, &p);
                if (isDeviceSuitable(d) && p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    m_physicalDevice = d;
                    m_queueFamilies  = findQueueFamilies(d);
                    return;
                }
            }
            for (const auto &d : devices) {
                if (isDeviceSuitable(d)) {
                    m_physicalDevice = d;
                    m_queueFamilies  = findQueueFamilies(d);
                    return;
                }
            }
            throw std::runtime_error("Vulkan: no suitable GPU found");
        }

        bool VulkanBackend::isDeviceSuitable(VkPhysicalDevice device) const {
            if (!findQueueFamilies(device).isComplete()) return false;

            uint32_t extCount = 0;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
            std::vector<VkExtensionProperties> available(extCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, available.data());

            for (const char *req : k_deviceExtensions) {
                bool found = false;
                for (const auto &ext : available)
                    if (strcmp(ext.extensionName, req) == 0) { found = true; break; }
                if (!found) return false;
            }

            // Verify swapchain has at least one surface format and present mode
            auto sc = querySwapchainSupport(device);
            return !sc.formats.empty() && !sc.presentModes.empty();
        }

        void VulkanBackend::createLogicalDevice() {
            std::set<uint32_t> unique = {
                m_queueFamilies.graphicsFamily.value(),
                m_queueFamilies.computeFamily.value(),
                m_queueFamilies.transferFamily.value(),
                m_queueFamilies.presentFamily.value()
            };

            const float prio = 1.0f;
            std::vector<VkDeviceQueueCreateInfo> queueCIs;
            queueCIs.reserve(unique.size());
            for (uint32_t f : unique) {
                VkDeviceQueueCreateInfo qci{};
                qci.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                qci.queueFamilyIndex = f;
                qci.queueCount       = 1;
                qci.pQueuePriorities = &prio;
                queueCIs.push_back(qci);
            }

            VkPhysicalDeviceFeatures features{};

            VkDeviceCreateInfo ci{};
            ci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            ci.queueCreateInfoCount    = static_cast<uint32_t>(queueCIs.size());
            ci.pQueueCreateInfos       = queueCIs.data();
            ci.enabledExtensionCount   = static_cast<uint32_t>(k_deviceExtensions.size());
            ci.ppEnabledExtensionNames = k_deviceExtensions.data();
            ci.pEnabledFeatures        = &features;

            if (k_enableValidation) {
                ci.enabledLayerCount   = static_cast<uint32_t>(k_validationLayers.size());
                ci.ppEnabledLayerNames = k_validationLayers.data();
            }

            if (vkCreateDevice(m_physicalDevice, &ci, nullptr, &m_device) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create logical device");

            vkGetDeviceQueue(m_device, m_queueFamilies.graphicsFamily.value(), 0, &m_graphicsQueue);
            vkGetDeviceQueue(m_device, m_queueFamilies.computeFamily.value(),  0, &m_computeQueue);
            vkGetDeviceQueue(m_device, m_queueFamilies.transferFamily.value(), 0, &m_transferQueue);
            vkGetDeviceQueue(m_device, m_queueFamilies.presentFamily.value(),  0, &m_presentQueue);
        }

        // ============================================================
        // F-10: VMA allocator
        // ============================================================

        void VulkanBackend::createAllocator() {
            VmaAllocatorCreateInfo ai{};
            ai.instance       = m_instance;
            ai.physicalDevice = m_physicalDevice;
            ai.device         = m_device;
            if (vmaCreateAllocator(&ai, &m_allocator) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create VMA allocator");

            m_resourceManager = std::make_unique<VulkanResourceManager>(m_device, m_allocator);
        }

        // ============================================================
        // F-07: Command pool
        // ============================================================

        void VulkanBackend::createCommandPool() {
            VkCommandPoolCreateInfo ci{};
            ci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            ci.queueFamilyIndex = m_queueFamilies.graphicsFamily.value();
            ci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            if (vkCreateCommandPool(m_device, &ci, nullptr, &m_commandPool) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create command pool");
        }

        // ============================================================
        // F-07: Swapchain recreation
        // ============================================================

        void VulkanBackend::cleanupSwapchain() {
            for (auto fb : m_imguiFramebuffers)
                vkDestroyFramebuffer(m_device, fb, nullptr);
            m_imguiFramebuffers.clear();

            for (auto fb : m_geometryFramebuffers)
                vkDestroyFramebuffer(m_device, fb, nullptr);
            m_geometryFramebuffers.clear();

            if (m_depthImageView != VK_NULL_HANDLE) {
                vkDestroyImageView(m_device, m_depthImageView, nullptr);
                m_depthImageView = VK_NULL_HANDLE;
            }
            if (m_depthImage != VK_NULL_HANDLE) {
                vmaDestroyImage(m_allocator, m_depthImage, m_depthAlloc);
                m_depthImage = VK_NULL_HANDLE;
                m_depthAlloc = VK_NULL_HANDLE;
            }

            for (auto iv : m_swapchainImageViews)
                vkDestroyImageView(m_device, iv, nullptr);
            m_swapchainImageViews.clear();

            if (m_imguiRenderPass != VK_NULL_HANDLE) {
                vkDestroyRenderPass(m_device, m_imguiRenderPass, nullptr);
                m_imguiRenderPass = VK_NULL_HANDLE;
            }
            if (m_geometryRenderPass != VK_NULL_HANDLE) {
                vkDestroyRenderPass(m_device, m_geometryRenderPass, nullptr);
                m_geometryRenderPass = VK_NULL_HANDLE;
            }

            if (m_swapchain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
                m_swapchain = VK_NULL_HANDLE;
            }
        }

        void VulkanBackend::recreateSwapchain() {
            vkDeviceWaitIdle(m_device);
            cleanupSwapchain();

            createSwapchain();
            createSwapchainImageViews();
            createGeometryRenderPass();
            createImGuiRenderPass();
            createDepthResources();
            createFramebuffers();

            // Update ImGui context with new render pass
            m_imguiContext.imguiRenderPass = m_imguiRenderPass;
            m_imguiContext.imageCount      = m_swapchainImageCount;
        }

        void VulkanBackend::createSwapchain() {
            SwapchainSupportDetails sc = querySwapchainSupport(m_physicalDevice);
            VkSurfaceFormatKHR fmt     = chooseSwapSurfaceFormat(sc.formats);
            VkPresentModeKHR   pm      = chooseSwapPresentMode(sc.presentModes);
            VkExtent2D         ext     = chooseSwapExtent(sc.capabilities);

            uint32_t imgCount = sc.capabilities.minImageCount + 1;
            if (sc.capabilities.maxImageCount > 0)
                imgCount = std::min(imgCount, sc.capabilities.maxImageCount);

            VkSwapchainCreateInfoKHR ci{};
            ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            ci.surface          = m_surface;
            ci.minImageCount    = imgCount;
            ci.imageFormat      = fmt.format;
            ci.imageColorSpace  = fmt.colorSpace;
            ci.imageExtent      = ext;
            ci.imageArrayLayers = 1;
            ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            uint32_t queueFamilyIndices[] = {
                m_queueFamilies.graphicsFamily.value(),
                m_queueFamilies.presentFamily.value()
            };

            if (m_queueFamilies.graphicsFamily != m_queueFamilies.presentFamily) {
                ci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
                ci.queueFamilyIndexCount = 2;
                ci.pQueueFamilyIndices   = queueFamilyIndices;
            } else {
                ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            ci.preTransform   = sc.capabilities.currentTransform;
            ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            ci.presentMode    = pm;
            ci.clipped        = VK_TRUE;
            ci.oldSwapchain   = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(m_device, &ci, nullptr, &m_swapchain) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create swapchain");

            vkGetSwapchainImagesKHR(m_device, m_swapchain, &imgCount, nullptr);
            m_swapchainImages.resize(imgCount);
            vkGetSwapchainImagesKHR(m_device, m_swapchain, &imgCount, m_swapchainImages.data());

            m_swapchainFormat     = fmt.format;
            m_swapchainExtent     = ext;
            m_swapchainImageCount = imgCount;
        }

        void VulkanBackend::createSwapchainImageViews() {
            m_swapchainImageViews.resize(m_swapchainImages.size());
            for (size_t i = 0; i < m_swapchainImages.size(); ++i)
                m_swapchainImageViews[i] = createImageView(
                    m_swapchainImages[i], m_swapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }

        void VulkanBackend::createGeometryRenderPass() {
            VkAttachmentDescription colorAtt{};
            colorAtt.format         = m_swapchainFormat;
            colorAtt.samples        = VK_SAMPLE_COUNT_1_BIT;
            colorAtt.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAtt.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            colorAtt.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAtt.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAtt.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentDescription depthAtt{};
            depthAtt.format         = findDepthFormat();
            depthAtt.samples        = VK_SAMPLE_COUNT_1_BIT;
            depthAtt.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAtt.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAtt.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAtt.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAtt.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
            VkAttachmentReference depthRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount    = 1;
            subpass.pColorAttachments       = &colorRef;
            subpass.pDepthStencilAttachment = &depthRef;

            VkSubpassDependency dep{};
            dep.srcSubpass    = VK_SUBPASS_EXTERNAL;
            dep.dstSubpass    = 0;
            dep.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                              | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dep.srcAccessMask = 0;
            dep.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                              | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                              | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            std::array<VkAttachmentDescription, 2> atts = {colorAtt, depthAtt};
            VkRenderPassCreateInfo rpCI{};
            rpCI.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rpCI.attachmentCount = static_cast<uint32_t>(atts.size());
            rpCI.pAttachments    = atts.data();
            rpCI.subpassCount    = 1;
            rpCI.pSubpasses      = &subpass;
            rpCI.dependencyCount = 1;
            rpCI.pDependencies   = &dep;

            if (vkCreateRenderPass(m_device, &rpCI, nullptr, &m_geometryRenderPass) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create geometry render pass");
        }

        void VulkanBackend::createImGuiRenderPass() {
            // LOAD_OP_LOAD — preserve the geometry output; ImGui draws on top.
            // Final layout is PRESENT_SRC_KHR so the image is ready for presentation.
            VkAttachmentDescription colorAtt{};
            colorAtt.format         = m_swapchainFormat;
            colorAtt.samples        = VK_SAMPLE_COUNT_1_BIT;
            colorAtt.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAtt.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            colorAtt.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAtt.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAtt.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments    = &colorRef;

            VkSubpassDependency dep{};
            dep.srcSubpass    = VK_SUBPASS_EXTERNAL;
            dep.dstSubpass    = 0;
            dep.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dep.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo rpCI{};
            rpCI.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rpCI.attachmentCount = 1;
            rpCI.pAttachments    = &colorAtt;
            rpCI.subpassCount    = 1;
            rpCI.pSubpasses      = &subpass;
            rpCI.dependencyCount = 1;
            rpCI.pDependencies   = &dep;

            if (vkCreateRenderPass(m_device, &rpCI, nullptr, &m_imguiRenderPass) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create ImGui render pass");
        }

        void VulkanBackend::createDepthResources() {
            m_depthFormat = findDepthFormat();
            createImage(m_swapchainExtent.width, m_swapchainExtent.height,
                        m_depthFormat, VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                        VMA_MEMORY_USAGE_GPU_ONLY,
                        m_depthImage, m_depthAlloc);
            m_depthImageView = createImageView(m_depthImage, m_depthFormat,
                                               VK_IMAGE_ASPECT_DEPTH_BIT);
        }

        void VulkanBackend::createFramebuffers() {
            const size_t n = m_swapchainImageViews.size();
            m_geometryFramebuffers.resize(n);
            m_imguiFramebuffers.resize(n);

            for (size_t i = 0; i < n; ++i) {
                // Geometry framebuffer (colour + depth)
                std::array<VkImageView, 2> geomAttachments = {
                    m_swapchainImageViews[i], m_depthImageView
                };
                VkFramebufferCreateInfo fbCI{};
                fbCI.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                fbCI.renderPass      = m_geometryRenderPass;
                fbCI.attachmentCount = static_cast<uint32_t>(geomAttachments.size());
                fbCI.pAttachments    = geomAttachments.data();
                fbCI.width           = m_swapchainExtent.width;
                fbCI.height          = m_swapchainExtent.height;
                fbCI.layers          = 1;

                if (vkCreateFramebuffer(m_device, &fbCI, nullptr,
                                        &m_geometryFramebuffers[i]) != VK_SUCCESS)
                    throw std::runtime_error("Vulkan: failed to create geometry framebuffer");

                // ImGui framebuffer (colour only, no depth)
                VkFramebufferCreateInfo imguiFbCI{};
                imguiFbCI.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                imguiFbCI.renderPass      = m_imguiRenderPass;
                imguiFbCI.attachmentCount = 1;
                imguiFbCI.pAttachments    = &m_swapchainImageViews[i];
                imguiFbCI.width           = m_swapchainExtent.width;
                imguiFbCI.height          = m_swapchainExtent.height;
                imguiFbCI.layers          = 1;

                if (vkCreateFramebuffer(m_device, &imguiFbCI, nullptr,
                                        &m_imguiFramebuffers[i]) != VK_SUCCESS)
                    throw std::runtime_error("Vulkan: failed to create ImGui framebuffer");
            }
        }

        void VulkanBackend::createSyncObjects() {
            VkSemaphoreCreateInfo semCI{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VkFenceCreateInfo     fenceCI{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for (int i = 0; i < k_maxFramesInFlight; ++i) {
                if (vkCreateSemaphore(m_device, &semCI, nullptr,
                                      &m_imageAvailableSemaphores[i]) != VK_SUCCESS
                    || vkCreateSemaphore(m_device, &semCI, nullptr,
                                         &m_renderFinishedSemaphores[i]) != VK_SUCCESS
                    || vkCreateFence(m_device, &fenceCI, nullptr,
                                     &m_inFlightFences[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("Vulkan: failed to create sync objects");
                }
            }
        }

        void VulkanBackend::createCommandBuffers() {
            VkCommandBufferAllocateInfo ai{};
            ai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            ai.commandPool        = m_commandPool;
            ai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            ai.commandBufferCount = k_maxFramesInFlight;

            if (vkAllocateCommandBuffers(m_device, &ai, m_commandBuffers.data()) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to allocate command buffers");
        }

        // ============================================================
        // F-08: Descriptor pool + layout + sets + UBOs
        // ============================================================

        void VulkanBackend::createDescriptorPool() {
            std::array<VkDescriptorPoolSize, 2> poolSizes{};
            poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(k_maxFramesInFlight) + 100;
            poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = 100;

            VkDescriptorPoolCreateInfo ci{};
            ci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            ci.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            ci.pPoolSizes    = poolSizes.data();
            ci.maxSets       = static_cast<uint32_t>(k_maxFramesInFlight) + 100;
            ci.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

            if (vkCreateDescriptorPool(m_device, &ci, nullptr, &m_descriptorPool) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create descriptor pool");
        }

        void VulkanBackend::createDescriptorSetLayout() {
            VkDescriptorSetLayoutBinding uboBinding{};
            uboBinding.binding            = 0;
            uboBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboBinding.descriptorCount    = 1;
            uboBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutCreateInfo ci{};
            ci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            ci.bindingCount = 1;
            ci.pBindings    = &uboBinding;

            if (vkCreateDescriptorSetLayout(m_device, &ci, nullptr, &m_descSetLayout) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create descriptor set layout");
        }

        void VulkanBackend::createUBOs() {
            for (int i = 0; i < k_maxFramesInFlight; ++i) {
                m_uboBuffers[i] = m_resourceManager->createHostBuffer(
                    sizeof(ViewProjUBO),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

                vmaMapMemory(m_allocator, m_uboBuffers[i].allocation, &m_uboMapped[i]);
            }
        }

        void VulkanBackend::createDescriptorSets() {
            std::array<VkDescriptorSetLayout, k_maxFramesInFlight> layouts;
            layouts.fill(m_descSetLayout);

            VkDescriptorSetAllocateInfo ai{};
            ai.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            ai.descriptorPool     = m_descriptorPool;
            ai.descriptorSetCount = k_maxFramesInFlight;
            ai.pSetLayouts        = layouts.data();

            if (vkAllocateDescriptorSets(m_device, &ai, m_descriptorSets.data()) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to allocate descriptor sets");

            for (int i = 0; i < k_maxFramesInFlight; ++i) {
                VkDescriptorBufferInfo bufInfo{};
                bufInfo.buffer = m_uboBuffers[i].buffer;
                bufInfo.offset = 0;
                bufInfo.range  = sizeof(ViewProjUBO);

                VkWriteDescriptorSet write{};
                write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet          = m_descriptorSets[i];
                write.dstBinding      = 0;
                write.dstArrayElement = 0;
                write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo     = &bufInfo;

                vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
            }
        }

        // ============================================================
        // F-08: Graphics pipeline
        // ============================================================

        void VulkanBackend::createGeometryPipeline() {
            // Shader modules from embedded SPIR-V stubs
            VkShaderModule vertModule = createShaderModule(
                k_geometryVertSPIRV, k_geometryVertSPIRVSize);
            VkShaderModule fragModule = createShaderModule(
                k_geometryFragSPIRV, k_geometryFragSPIRVSize);

            VkPipelineShaderStageCreateInfo stages[2]{};
            stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
            stages[0].module = vertModule;
            stages[0].pName  = "main";

            stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            stages[1].module = fragModule;
            stages[1].pName  = "main";

            // Vertex input: position(vec3), normal(vec3), uv(vec2)
            VkVertexInputBindingDescription binding{};
            binding.binding   = 0;
            binding.stride    = sizeof(Mesh::Vertex);
            binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            std::array<VkVertexInputAttributeDescription, 3> attrs{};
            attrs[0].binding  = 0;
            attrs[0].location = 0;
            attrs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attrs[0].offset   = offsetof(Mesh::Vertex, position);

            attrs[1].binding  = 0;
            attrs[1].location = 1;
            attrs[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attrs[1].offset   = offsetof(Mesh::Vertex, normal);

            attrs[2].binding  = 0;
            attrs[2].location = 2;
            attrs[2].format   = VK_FORMAT_R32G32_SFLOAT;
            attrs[2].offset   = offsetof(Mesh::Vertex, uv);

            VkPipelineVertexInputStateCreateInfo vertexInput{};
            vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInput.vertexBindingDescriptionCount   = 1;
            vertexInput.pVertexBindingDescriptions      = &binding;
            vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrs.size());
            vertexInput.pVertexAttributeDescriptions    = attrs.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount  = 1;

            VkPipelineRasterizationStateCreateInfo rast{};
            rast.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rast.polygonMode = VK_POLYGON_MODE_FILL;
            rast.cullMode    = VK_CULL_MODE_BACK_BIT;
            rast.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rast.lineWidth   = 1.0f;

            VkPipelineMultisampleStateCreateInfo ms{};
            ms.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineDepthStencilStateCreateInfo ds{};
            ds.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            ds.depthTestEnable  = VK_TRUE;
            ds.depthWriteEnable = VK_TRUE;
            ds.depthCompareOp   = VK_COMPARE_OP_LESS;

            VkPipelineColorBlendAttachmentState blendAtt{};
            blendAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                    | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            blendAtt.blendEnable    = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo blend{};
            blend.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            blend.attachmentCount   = 1;
            blend.pAttachments      = &blendAtt;

            std::array<VkDynamicState, 2> dynStates = {
                VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
            };
            VkPipelineDynamicStateCreateInfo dynState{};
            dynState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynState.dynamicStateCount = static_cast<uint32_t>(dynStates.size());
            dynState.pDynamicStates    = dynStates.data();

            // Push constant: model matrix (mat4 = 64 bytes)
            VkPushConstantRange pcRange{};
            pcRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pcRange.offset     = 0;
            pcRange.size       = sizeof(glm::mat4);

            VkPipelineLayoutCreateInfo layoutCI{};
            layoutCI.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layoutCI.setLayoutCount         = 1;
            layoutCI.pSetLayouts            = &m_descSetLayout;
            layoutCI.pushConstantRangeCount = 1;
            layoutCI.pPushConstantRanges    = &pcRange;

            if (vkCreatePipelineLayout(m_device, &layoutCI, nullptr,
                                       &m_geometryPipelineLayout) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create geometry pipeline layout");

            VkGraphicsPipelineCreateInfo pipeCI{};
            pipeCI.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeCI.stageCount          = 2;
            pipeCI.pStages             = stages;
            pipeCI.pVertexInputState   = &vertexInput;
            pipeCI.pInputAssemblyState = &inputAssembly;
            pipeCI.pViewportState      = &viewportState;
            pipeCI.pRasterizationState = &rast;
            pipeCI.pMultisampleState   = &ms;
            pipeCI.pDepthStencilState  = &ds;
            pipeCI.pColorBlendState    = &blend;
            pipeCI.pDynamicState       = &dynState;
            pipeCI.layout              = m_geometryPipelineLayout;
            pipeCI.renderPass          = m_geometryRenderPass;
            pipeCI.subpass             = 0;

            VkResult r = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1,
                                                   &pipeCI, nullptr,
                                                   &m_geometryPipeline);

            vkDestroyShaderModule(m_device, vertModule, nullptr);
            vkDestroyShaderModule(m_device, fragModule, nullptr);

            if (r != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create geometry graphics pipeline");

            m_geometryPipelineReady = true;
        }

        // ============================================================
        // F-08: Mesh upload
        // ============================================================

        void VulkanBackend::uploadMesh(const Mesh &mesh) {
            GpuMesh gm{};
            gm.vertexCount = static_cast<uint32_t>(mesh.vertices().size());
            gm.indexCount  = static_cast<uint32_t>(mesh.indices().size());

            if (gm.vertexCount == 0) return;

            // Vertex buffer
            VkDeviceSize vbSize = gm.vertexCount * sizeof(Mesh::Vertex);
            gm.vertexBuffer = m_resourceManager->createDeviceBuffer(
                vbSize,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                mesh.vertices().data(),
                m_commandPool,
                m_graphicsQueue);

            // Index buffer (optional)
            if (gm.indexCount > 0) {
                VkDeviceSize ibSize = gm.indexCount * sizeof(uint32_t);
                gm.indexBuffer = m_resourceManager->createDeviceBuffer(
                    ibSize,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    mesh.indices().data(),
                    m_commandPool,
                    m_graphicsQueue);
            }

            m_gpuMeshes[mesh.getName()] = std::move(gm);
        }

        // ============================================================
        // F-11: Compute pipeline
        // ============================================================

        void VulkanBackend::createComputePipeline() {
            m_computePipeline = std::make_unique<VulkanComputePipeline>(m_device, m_allocator);

            // Use the physics integration SPIR-V stub.
            // Replace with the output of:
            //   glslangValidator -V assets/shaders/compute/physics_integrate.comp \
            //                   -o physics_integrate.spv
            // and embed the resulting .spv as a uint32_t[] array in PhysicsShaderSPIRV.h.
            std::vector<uint32_t> spirv(k_physicsIntegrateSPIRV,
                                        k_physicsIntegrateSPIRV + k_physicsIntegrateSPIRVSize);

            // Descriptor: one storage buffer binding
            VkDescriptorSetLayoutBinding ssboBinding{};
            ssboBinding.binding         = 0;
            ssboBinding.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            ssboBinding.descriptorCount = 1;
            ssboBinding.stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;

            // Push constant: gravity(vec3) + deltaTime(float) + bodyCount(uint) = 20 bytes,
            // padded to 24 for alignment.
            struct PhysicsPushConstants {
                glm::vec3 gravity;
                float     deltaTime;
                uint32_t  bodyCount;
                uint32_t  _pad;
            };

            try {
                m_computePipeline->create(spirv, {ssboBinding},
                                          sizeof(PhysicsPushConstants));
            } catch (const std::exception &) {
                // Compute pipeline creation may fail if the SPIR-V stub is incomplete.
                // Log and continue — physics will fall back to CPU.
                m_computePipeline.reset();
            }

            // Physics descriptor pool + set layout
            createPhysicsDescriptorSetLayout();
        }

        void VulkanBackend::createPhysicsDescriptorSetLayout() {
            VkDescriptorSetLayoutBinding b{};
            b.binding         = 0;
            b.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            b.descriptorCount = 1;
            b.stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;

            VkDescriptorSetLayoutCreateInfo ci{};
            ci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            ci.bindingCount = 1;
            ci.pBindings    = &b;

            if (vkCreateDescriptorSetLayout(m_device, &ci, nullptr,
                                            &m_physicsDescSetLayout) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create physics descriptor set layout");

            // Create a small descriptor pool for the physics SSBO set
            VkDescriptorPoolSize ps{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1};
            VkDescriptorPoolCreateInfo poolCI{};
            poolCI.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCI.poolSizeCount = 1;
            poolCI.pPoolSizes    = &ps;
            poolCI.maxSets       = 1;

            if (vkCreateDescriptorPool(m_device, &poolCI, nullptr,
                                       &m_physicsDescPool) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create physics descriptor pool");
        }

        void VulkanBackend::createPhysicsDescriptorSet(VkDeviceSize ssboSize) {
            // Allocate descriptor set
            VkDescriptorSetAllocateInfo ai{};
            ai.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            ai.descriptorPool     = m_physicsDescPool;
            ai.descriptorSetCount = 1;
            ai.pSetLayouts        = &m_physicsDescSetLayout;

            if (vkAllocateDescriptorSets(m_device, &ai, &m_physicsDescSet) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to allocate physics descriptor set");

            // Bind the SSBO
            VkDescriptorBufferInfo bufInfo{};
            bufInfo.buffer = m_physicsBuffer.buffer;
            bufInfo.offset = 0;
            bufInfo.range  = ssboSize;

            VkWriteDescriptorSet w{};
            w.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            w.dstSet          = m_physicsDescSet;
            w.dstBinding      = 0;
            w.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            w.descriptorCount = 1;
            w.pBufferInfo     = &bufInfo;

            vkUpdateDescriptorSets(m_device, 1, &w, 0, nullptr);
        }

        // ============================================================
        // F-12/F-13: GPU physics dispatch
        // ============================================================

        // GPU-side physics body layout must match the compute shader struct.
        struct GpuPhysicsBody {
            glm::vec3 position;    float _pad0;
            glm::vec3 velocity;    float _pad1;
            glm::vec3 acceleration; float _pad2;
            float mass;
            int   isStatic;
            int   useGravity;
            float _pad3;
        };

        void VulkanBackend::dispatchPhysics(
            std::vector<glm::vec3> &positions,
            std::vector<glm::vec3> &velocities,
            std::vector<glm::vec3> &accelerations,
            const std::vector<float> &masses,
            const std::vector<bool> &isStatic,
            const std::vector<bool> &useGravity,
            glm::vec3 gravity,
            float deltaTime)
        {
            if (!m_computePipeline) return; // Fall back to CPU

            const uint32_t bodyCount = static_cast<uint32_t>(positions.size());
            if (bodyCount == 0) return;

            const VkDeviceSize ssboSize = bodyCount * sizeof(GpuPhysicsBody);

            // --- Create / resize SSBO ---
            if (!m_physicsBufferReady || m_physicsBodyCount != bodyCount) {
                if (m_physicsBufferReady) {
                    m_resourceManager->destroyBuffer(m_physicsBuffer);
                    m_resourceManager->destroyBuffer(m_physicsReadbackBuffer);
                    // Re-create descriptor set (pool must allow it)
                    vkResetDescriptorPool(m_device, m_physicsDescPool, 0);
                }

                // Persistent SSBO (host-visible so we can write/read without a staging copy)
                m_physicsBuffer = m_resourceManager->createHostBuffer(
                    ssboSize,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                    | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                    | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

                m_physicsReadbackBuffer = m_resourceManager->createHostBuffer(
                    ssboSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT);

                createPhysicsDescriptorSet(ssboSize);

                m_physicsBodyCount  = bodyCount;
                m_physicsBufferReady = true;
            }

            // --- Upload current CPU state to SSBO ---
            void *mapped = nullptr;
            vmaMapMemory(m_allocator, m_physicsBuffer.allocation, &mapped);
            auto *gpuBodies = reinterpret_cast<GpuPhysicsBody *>(mapped);
            for (uint32_t i = 0; i < bodyCount; ++i) {
                gpuBodies[i].position     = positions[i];
                gpuBodies[i].velocity     = velocities[i];
                gpuBodies[i].acceleration = accelerations[i];
                gpuBodies[i].mass         = masses[i];
                gpuBodies[i].isStatic     = isStatic[i] ? 1 : 0;
                gpuBodies[i].useGravity   = useGravity[i] ? 1 : 0;
                gpuBodies[i]._pad0 = gpuBodies[i]._pad1 =
                gpuBodies[i]._pad2 = gpuBodies[i]._pad3 = 0.0f;
            }
            vmaUnmapMemory(m_allocator, m_physicsBuffer.allocation);

            // --- Dispatch compute ---
            // One-shot command buffer on the compute queue
            VkCommandBufferAllocateInfo allocAI{};
            allocAI.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocAI.commandPool        = m_commandPool;
            allocAI.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocAI.commandBufferCount = 1;

            VkCommandBuffer cmd;
            vkAllocateCommandBuffers(m_device, &allocAI, &cmd);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &beginInfo);

            struct PhysicsPushConstants {
                glm::vec3 gravity;
                float     deltaTime;
                uint32_t  bodyCount;
                uint32_t  _pad;
            } pc{ gravity, deltaTime, bodyCount, 0 };

            if (m_computePipeline && m_computePipeline->pipeline() != VK_NULL_HANDLE) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                                  m_computePipeline->pipeline());
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                                        m_computePipeline->layout(), 0, 1,
                                        &m_physicsDescSet, 0, nullptr);
                vkCmdPushConstants(cmd, m_computePipeline->layout(),
                                   VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                   sizeof(PhysicsPushConstants), &pc);

                // Dispatch: ceil(bodyCount / 64) groups
                uint32_t groups = (bodyCount + 63) / 64;
                vkCmdDispatch(cmd, groups, 1, 1);
            }

            vkEndCommandBuffer(cmd);

            VkSubmitInfo submitInfo{};
            submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = &cmd;

            vkQueueSubmit(m_computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(m_computeQueue);

            vkFreeCommandBuffers(m_device, m_commandPool, 1, &cmd);

            // --- Read back positions immediately for this frame ---
            // (Full readback at reduced rate is done in readbackPhysicsPositions)
            vmaMapMemory(m_allocator, m_physicsBuffer.allocation, &mapped);
            gpuBodies = reinterpret_cast<GpuPhysicsBody *>(mapped);
            for (uint32_t i = 0; i < bodyCount; ++i) {
                positions[i]     = gpuBodies[i].position;
                velocities[i]    = gpuBodies[i].velocity;
                accelerations[i] = gpuBodies[i].acceleration;
            }
            vmaUnmapMemory(m_allocator, m_physicsBuffer.allocation);
        }

        void VulkanBackend::readbackPhysicsPositions(std::vector<glm::vec3> &positions) {
            // Only readback at reduced rate (~10 Hz)
            if (m_readbackFrameCounter % k_readbackInterval != 0) return;
            if (!m_physicsBufferReady) return;

            const uint32_t bodyCount = static_cast<uint32_t>(positions.size());
            if (bodyCount == 0 || bodyCount != m_physicsBodyCount) return;

            void *mapped = nullptr;
            vmaMapMemory(m_allocator, m_physicsBuffer.allocation, &mapped);
            auto *gpuBodies = reinterpret_cast<const GpuPhysicsBody *>(mapped);
            for (uint32_t i = 0; i < bodyCount; ++i)
                positions[i] = gpuBodies[i].position;
            vmaUnmapMemory(m_allocator, m_physicsBuffer.allocation);
        }

        // ============================================================
        // Utilities
        // ============================================================

        QueueFamilyIndices VulkanBackend::findQueueFamilies(VkPhysicalDevice device) const {
            QueueFamilyIndices indices;

            uint32_t count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
            std::vector<VkQueueFamilyProperties> families(count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

            for (uint32_t i = 0; i < count; ++i) {
                const auto &f = families[i];

                if (f.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    indices.graphicsFamily = i;

                if (f.queueFlags & VK_QUEUE_COMPUTE_BIT)
                    indices.computeFamily = i;

                if ((f.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                    !(f.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                    indices.transferFamily = i;

                VkBool32 present = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &present);
                if (present) indices.presentFamily = i;

                if (indices.isComplete()) break;
            }

            if (!indices.transferFamily.has_value() && indices.graphicsFamily.has_value())
                indices.transferFamily = indices.graphicsFamily;

            return indices;
        }

        bool VulkanBackend::checkValidationLayerSupport() const {
            uint32_t count = 0;
            vkEnumerateInstanceLayerProperties(&count, nullptr);
            std::vector<VkLayerProperties> available(count);
            vkEnumerateInstanceLayerProperties(&count, available.data());

            for (const char *req : k_validationLayers) {
                bool found = false;
                for (const auto &l : available)
                    if (strcmp(l.layerName, req) == 0) { found = true; break; }
                if (!found) return false;
            }
            return true;
        }

        std::vector<const char *> VulkanBackend::getRequiredExtensions() const {
            uint32_t glfwCount = 0;
            const char **glfwExts = glfwGetRequiredInstanceExtensions(&glfwCount);
            std::vector<const char *> exts(glfwExts, glfwExts + glfwCount);
            if (k_enableValidation) exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            return exts;
        }

        SwapchainSupportDetails VulkanBackend::querySwapchainSupport(VkPhysicalDevice device) const {
            SwapchainSupportDetails details;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

            uint32_t fmtCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &fmtCount, nullptr);
            if (fmtCount) {
                details.formats.resize(fmtCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &fmtCount,
                                                     details.formats.data());
            }

            uint32_t pmCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &pmCount, nullptr);
            if (pmCount) {
                details.presentModes.resize(pmCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &pmCount,
                                                          details.presentModes.data());
            }
            return details;
        }

        VkSurfaceFormatKHR VulkanBackend::chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &formats) const
        {
            for (const auto &f : formats)
                if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
                    f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return f;
            return formats[0];
        }

        VkPresentModeKHR VulkanBackend::chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &modes) const
        {
            for (const auto &m : modes)
                if (m == VK_PRESENT_MODE_MAILBOX_KHR) return m;
            return VK_PRESENT_MODE_FIFO_KHR; // Guaranteed to be available
        }

        VkExtent2D VulkanBackend::chooseSwapExtent(
            const VkSurfaceCapabilitiesKHR &caps) const
        {
            if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
                return caps.currentExtent;

            int w = 0, h = 0;
            glfwGetFramebufferSize(m_window, &w, &h);

            VkExtent2D ext{};
            ext.width  = std::clamp(static_cast<uint32_t>(w),
                                    caps.minImageExtent.width,
                                    caps.maxImageExtent.width);
            ext.height = std::clamp(static_cast<uint32_t>(h),
                                    caps.minImageExtent.height,
                                    caps.maxImageExtent.height);
            return ext;
        }

        VkFormat VulkanBackend::findDepthFormat() const {
            return findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                 VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        }

        VkFormat VulkanBackend::findSupportedFormat(
            const std::vector<VkFormat> &candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features) const
        {
            for (VkFormat fmt : candidates) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(m_physicalDevice, fmt, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR &&
                    (props.linearTilingFeatures & features) == features)
                    return fmt;
                if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                    (props.optimalTilingFeatures & features) == features)
                    return fmt;
            }
            throw std::runtime_error("Vulkan: failed to find supported format");
        }

        void VulkanBackend::createImage(uint32_t w, uint32_t h,
                                        VkFormat fmt, VkImageTiling tiling,
                                        VkImageUsageFlags usage,
                                        VmaMemoryUsage memUsage,
                                        VkImage &image, VmaAllocation &alloc)
        {
            VkImageCreateInfo ci{};
            ci.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            ci.imageType     = VK_IMAGE_TYPE_2D;
            ci.extent        = {w, h, 1};
            ci.mipLevels     = 1;
            ci.arrayLayers   = 1;
            ci.format        = fmt;
            ci.tiling        = tiling;
            ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            ci.usage         = usage;
            ci.samples       = VK_SAMPLE_COUNT_1_BIT;
            ci.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocCI{};
            allocCI.usage = memUsage;

            if (vmaCreateImage(m_allocator, &ci, &allocCI, &image, &alloc, nullptr) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create image");
        }

        VkImageView VulkanBackend::createImageView(VkImage image, VkFormat format,
                                                    VkImageAspectFlags aspect) const
        {
            VkImageViewCreateInfo ci{};
            ci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ci.image                           = image;
            ci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
            ci.format                          = format;
            ci.subresourceRange.aspectMask     = aspect;
            ci.subresourceRange.baseMipLevel   = 0;
            ci.subresourceRange.levelCount     = 1;
            ci.subresourceRange.baseArrayLayer = 0;
            ci.subresourceRange.layerCount     = 1;

            VkImageView view;
            if (vkCreateImageView(m_device, &ci, nullptr, &view) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create image view");
            return view;
        }

        VkShaderModule VulkanBackend::createShaderModule(const uint32_t *spv,
                                                          size_t wordCount) const
        {
            VkShaderModuleCreateInfo ci{};
            ci.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            ci.codeSize = wordCount * sizeof(uint32_t);
            ci.pCode    = spv;

            VkShaderModule mod;
            if (vkCreateShaderModule(m_device, &ci, nullptr, &mod) != VK_SUCCESS)
                throw std::runtime_error("Vulkan: failed to create shader module");
            return mod;
        }

    } // namespace Render
} // namespace Arche

#endif // ARCHE_BACKEND_VULKAN
