#pragma once
#ifdef ARCHE_BACKEND_VULKAN

#include "IRenderBackend.h"
#include "VulkanResourceManager.h"
#include "VulkanComputePipeline.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct GLFWwindow;

namespace Arche {
    namespace Render {

        /**
         * @brief Queue family indices for a selected physical device.
         *
         * Tracks which queue family indices support graphics, compute,
         * transfer, and present operations. All four must be present
         * for a device to be considered suitable.
         */
        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> computeFamily;
            std::optional<uint32_t> transferFamily;
            std::optional<uint32_t> presentFamily;

            bool isComplete() const {
                return graphicsFamily.has_value()
                    && computeFamily.has_value()
                    && transferFamily.has_value()
                    && presentFamily.has_value();
            }
        };

        /**
         * @brief Swapchain support details queried from the physical device.
         */
        struct SwapchainSupportDetails {
            VkSurfaceCapabilitiesKHR        capabilities{};
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR>   presentModes;
        };

        /**
         * @brief GPU mesh: device-local vertex + index buffers.
         */
        struct GpuMesh {
            GpuBuffer vertexBuffer;
            GpuBuffer indexBuffer;
            uint32_t  vertexCount{0};
            uint32_t  indexCount{0};
        };

        /**
         * @brief Per-frame UBO data (view + projection matrices).
         */
        struct ViewProjUBO {
            glm::mat4 view{1.0f};
            glm::mat4 proj{1.0f};
        };

        /**
         * @brief Vulkan context handles exposed to the ImGui integration.
         *
         * Populated by VulkanBackend::initialise() and kept valid for the engine lifetime.
         * Access via VulkanBackend::getVulkanContextForImGui().
         */
        struct VulkanContextForImGui {
            VkInstance       instance{VK_NULL_HANDLE};
            VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
            VkDevice         device{VK_NULL_HANDLE};
            VkQueue          graphicsQueue{VK_NULL_HANDLE};
            uint32_t         graphicsQueueFamily{0};           ///< graphics queue family index
            VkRenderPass     imguiRenderPass{VK_NULL_HANDLE};  ///< separate ImGui render pass
            uint32_t         minImageCount{2};
            uint32_t         imageCount{2};
        };

        /**
         * @brief Vulkan rendering backend implementing IRenderBackend.
         *
         * Implements F-06 through F-13:
         *   F-06: instance, surface, physical device, logical device, queues
         *   F-07: swapchain, render passes, framebuffers, sync, frame loop
         *   F-08: geometry pipeline, mesh upload/draw, view-projection UBO
         *   F-09: ImGui integration hooks (render pass + context struct)
         *   F-10: VulkanResourceManager + VMA allocator
         *   F-11: VulkanComputePipeline
         *   F-12: GPU physics dispatch
         *   F-13: persistent physics SSBO, partial CPU readback
         *
         * @note Requires GLFW window created with GLFW_CLIENT_API = GLFW_NO_API.
         */
        class VulkanBackend : public IRenderBackend {
          public:
            explicit VulkanBackend(GLFWwindow *window);
            ~VulkanBackend() noexcept override { shutdown(); }

            // --- Lifecycle ---
            void initialise() override;
            void shutdown() noexcept override;
            void resize(glm::ivec2 newSize) override;

            // --- Frame ---
            void beginFrame() override;
            void endFrame() override;

            // --- Rendering ---
            void setViewProjection(const glm::mat4 &view, const glm::mat4 &proj) override;
            void drawMesh(const Mesh &mesh, const glm::mat4 &model) override;

            // --- Stubs (not required for Vulkan MVP) ---
            void     setShader(std::shared_ptr<Shader>) override {}
            void     setMaterial(const Material &) override {}
            void     bindTexture(const std::string &, unsigned int, int) override {}
            void     setUniformMat4(const std::string &, const glm::mat4 &) override {}
            void     setUniformVec4(const std::string &, const glm::vec4 &) override {}
            void     setUniform1f(const std::string &, float) override {}
            void     setUniform1i(const std::string &, int) override {}
            void     setUniformVec3(const std::string &, const glm::vec3 &) override {}
            void     setDepthMask(bool) override {}
            void     setWireframe(bool) override {}
            void     initialiseShadowResources(int) override {}
            void     beginShadowPass() override {}
            void     endShadowPass() override {}
            void     drawMeshDepthOnly(const Mesh &, const glm::mat4 &) override {}
            void     beginDebugLines() override {}
            void     drawDebugLine(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &) override {}
            void     endDebugLines() override {}
            unsigned int getRenderTextureID() const override { return 0; }
            unsigned int getShadowMapTextureID() const override { return 0; }

            // --- F-09: ImGui integration ---

            /**
             * @brief Return the Vulkan context struct needed to initialise ImGui Vulkan backend.
             *
             * The returned pointer is valid for the lifetime of this VulkanBackend.
             */
            const VulkanContextForImGui *getVulkanContextForImGui() const {
                return &m_imguiContext;
            }

            /**
             * @brief Register a callback invoked inside endFrame() to render ImGui draw data.
             *
             * ImGuiBackend should set this so that ImGui rendering is driven inside the
             * VulkanBackend frame loop (after the geometry pass, inside the ImGui render pass).
             */
            void setImGuiRenderCallback(std::function<void(VkCommandBuffer)> cb) {
                m_imguiRenderCallback = std::move(cb);
            }

            /**
             * @brief Return the ImGui render pass (LOAD_OP_LOAD, suitable for rendering on top).
             */
            VkRenderPass getImGuiRenderPass() const { return m_imguiRenderPass; }

            // --- F-12/F-13: GPU physics dispatch ---

            /**
             * @brief Dispatch physics integration on the GPU.
             *
             * On the first call the physics SSBO is created. Each subsequent call
             * uploads changed bodies and dispatches the compute shader.
             *
             * @param positions     Body positions (read/write; updated after dispatch)
             * @param velocities    Body velocities (read/write)
             * @param accelerations Body accelerations (read/write)
             * @param masses        Body masses (read-only)
             * @param isStatic      Static flags (read-only)
             * @param useGravity    Gravity flags (read-only)
             * @param gravity       Gravity vector
             * @param deltaTime     Integration time step
             */
            void dispatchPhysics(
                std::vector<glm::vec3> &positions,
                std::vector<glm::vec3> &velocities,
                std::vector<glm::vec3> &accelerations,
                const std::vector<float> &masses,
                const std::vector<bool> &isStatic,
                const std::vector<bool> &useGravity,
                glm::vec3 gravity,
                float deltaTime);

            /**
             * @brief Read back physics positions from the GPU at reduced frequency.
             *
             * Should be called at ~10 Hz or when the simulation is paused so the
             * editor can display current positions without stalling every frame.
             *
             * @param[out] positions  Updated from the GPU SSBO
             */
            void readbackPhysicsPositions(std::vector<glm::vec3> &positions);

            /** @brief VkBuffer handle of the physics SSBO (for binding in the geometry pass). */
            VkBuffer     getPhysicsPositionBuffer()  const { return m_physicsBuffer.buffer; }
            VkDeviceSize getPhysicsPositionOffset()  const { return 0; }

          private:
            // --- F-06: Init helpers ---
            void createInstance();
            void setupDebugMessenger();
            void createSurface();
            void pickPhysicalDevice();
            void createLogicalDevice();

            // --- F-07: Swapchain helpers ---
            void createAllocator();                  ///< F-10 VMA init
            void createCommandPool();
            void recreateSwapchain();
            void cleanupSwapchain();
            void createSwapchain();
            void createSwapchainImageViews();
            void createGeometryRenderPass();
            void createImGuiRenderPass();
            void createDepthResources();
            void createFramebuffers();
            void createSyncObjects();
            void createCommandBuffers();

            // --- F-08: Geometry pipeline ---
            void createGeometryPipeline();
            void createDescriptorPool();
            void createDescriptorSetLayout();
            void createDescriptorSets();
            void createUBOs();
            void uploadMesh(const Mesh &mesh);

            // --- F-11: Compute pipeline ---
            void createComputePipeline();
            void createPhysicsDescriptorSetLayout();
            void createPhysicsDescriptorSet(VkDeviceSize ssboSize);

            // --- Utilities ---
            QueueFamilyIndices    findQueueFamilies(VkPhysicalDevice device) const;
            bool                  isDeviceSuitable(VkPhysicalDevice device) const;
            bool                  checkValidationLayerSupport() const;
            std::vector<const char *> getRequiredExtensions() const;
            SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device) const;
            VkSurfaceFormatKHR    chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &) const;
            VkPresentModeKHR      chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &) const;
            VkExtent2D            chooseSwapExtent(const VkSurfaceCapabilitiesKHR &) const;
            VkFormat              findDepthFormat() const;
            VkFormat              findSupportedFormat(const std::vector<VkFormat> &,
                                                      VkImageTiling,
                                                      VkFormatFeatureFlags) const;
            void createImage(uint32_t w, uint32_t h, VkFormat fmt,
                             VkImageTiling tiling, VkImageUsageFlags usage,
                             VmaMemoryUsage memUsage,
                             VkImage &image, VmaAllocation &alloc);
            VkImageView createImageView(VkImage image, VkFormat format,
                                        VkImageAspectFlags aspect) const;
            VkShaderModule createShaderModule(const uint32_t *spv, size_t wordCount) const;

            // --- F-06: Vulkan core handles ---
            GLFWwindow        *m_window{nullptr};
            VkInstance         m_instance{VK_NULL_HANDLE};
            VkSurfaceKHR       m_surface{VK_NULL_HANDLE};
            VkPhysicalDevice   m_physicalDevice{VK_NULL_HANDLE};
            VkDevice           m_device{VK_NULL_HANDLE};

            VkQueue            m_graphicsQueue{VK_NULL_HANDLE};
            VkQueue            m_computeQueue{VK_NULL_HANDLE};
            VkQueue            m_transferQueue{VK_NULL_HANDLE};
            VkQueue            m_presentQueue{VK_NULL_HANDLE};

            VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
            QueueFamilyIndices       m_queueFamilies;
            glm::ivec2               m_backBufferSize{800, 600};

            // --- F-10: VMA ---
            VmaAllocator                          m_allocator{VK_NULL_HANDLE};
            std::unique_ptr<VulkanResourceManager> m_resourceManager;

            // --- F-07: Swapchain ---
            VkSwapchainKHR             m_swapchain{VK_NULL_HANDLE};
            std::vector<VkImage>       m_swapchainImages;
            std::vector<VkImageView>   m_swapchainImageViews;
            VkFormat                   m_swapchainFormat{VK_FORMAT_UNDEFINED};
            VkExtent2D                 m_swapchainExtent{};
            uint32_t                   m_swapchainImageCount{0};

            // Depth buffer
            VkImage        m_depthImage{VK_NULL_HANDLE};
            VmaAllocation  m_depthAlloc{VK_NULL_HANDLE};
            VkImageView    m_depthImageView{VK_NULL_HANDLE};
            VkFormat       m_depthFormat{VK_FORMAT_D32_SFLOAT};

            // Render passes
            VkRenderPass m_geometryRenderPass{VK_NULL_HANDLE};
            VkRenderPass m_imguiRenderPass{VK_NULL_HANDLE};

            // Framebuffers (one per swapchain image, two sets: geometry + imgui)
            std::vector<VkFramebuffer> m_geometryFramebuffers;
            std::vector<VkFramebuffer> m_imguiFramebuffers;

            // Command pool + per-frame command buffers
            VkCommandPool                m_commandPool{VK_NULL_HANDLE};
            static constexpr int         k_maxFramesInFlight{2};
            std::array<VkCommandBuffer, k_maxFramesInFlight> m_commandBuffers{};

            // Per-frame sync
            std::array<VkSemaphore, k_maxFramesInFlight> m_imageAvailableSemaphores{};
            std::array<VkSemaphore, k_maxFramesInFlight> m_renderFinishedSemaphores{};
            std::array<VkFence,     k_maxFramesInFlight> m_inFlightFences{};

            // Frame state
            uint32_t m_currentFrame{0};
            uint32_t m_imageIndex{0};
            bool     m_framebufferResized{false};
            bool     m_frameStarted{false}; ///< true between a successful beginFrame and endFrame

            // --- F-08: Geometry pipeline ---
            VkDescriptorSetLayout m_descSetLayout{VK_NULL_HANDLE};
            VkPipelineLayout      m_geometryPipelineLayout{VK_NULL_HANDLE};
            VkPipeline            m_geometryPipeline{VK_NULL_HANDLE};
            VkDescriptorPool      m_descriptorPool{VK_NULL_HANDLE};
            std::array<VkDescriptorSet, k_maxFramesInFlight> m_descriptorSets{};
            std::array<GpuBuffer,       k_maxFramesInFlight> m_uboBuffers{};
            std::array<void *,          k_maxFramesInFlight> m_uboMapped{};

            bool m_geometryPipelineReady{false};

            // Mesh cache: meshName → GpuMesh
            std::unordered_map<std::string, GpuMesh> m_gpuMeshes;

            // Current frame view-projection
            ViewProjUBO m_viewProj{};

            // --- F-09: ImGui ---
            VulkanContextForImGui          m_imguiContext{};
            std::function<void(VkCommandBuffer)> m_imguiRenderCallback;

            // --- F-11: Compute pipeline ---
            std::unique_ptr<VulkanComputePipeline> m_computePipeline;

            // --- F-12/F-13: Physics GPU integration ---
            VkDescriptorSetLayout m_physicsDescSetLayout{VK_NULL_HANDLE};
            VkDescriptorPool      m_physicsDescPool{VK_NULL_HANDLE};
            VkDescriptorSet       m_physicsDescSet{VK_NULL_HANDLE};
            GpuBuffer             m_physicsBuffer;            ///< Persistent SSBO
            GpuBuffer             m_physicsReadbackBuffer;    ///< CPU-readable staging
            bool                  m_physicsBufferReady{false};
            uint32_t              m_physicsBodyCount{0};

            // CPU readback throttle: only readback every N frames
            uint32_t m_readbackFrameCounter{0};
            static constexpr uint32_t k_readbackInterval{6}; // ~10 Hz @ 60 fps

            // --- Constants ---
#ifdef NDEBUG
            static constexpr bool k_enableValidation{false};
#else
            static constexpr bool k_enableValidation{true};
#endif
            static const std::vector<const char *> k_validationLayers;
            static const std::vector<const char *> k_deviceExtensions;
        };

    } // namespace Render
} // namespace Arche

#endif // ARCHE_BACKEND_VULKAN
