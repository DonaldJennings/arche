#pragma once
#ifdef ARCHE_BACKEND_VULKAN

#include "IRenderBackend.h"
#include "VulkanResourceManager.h"
#include "VulkanComputePipeline.h"
#include "BvhBuilder.h"         // GpuSphere, GpuMaterial, GpuBvhNode

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
         * @brief Per-instance data written into the host-visible instance SSBO.
         *
         * std430 layout: mat4 (64 B) + uint32 (4 B) + uint32 (4 B) + pad (8 B) = 80 B.
         */
        struct GpuInstance {
            glm::mat4 transform{1.0f};  ///< Model-to-world matrix   (64 bytes)
            uint32_t  meshIndex{0};     ///< Index into m_meshRanges  ( 4 bytes)
            uint32_t  materialId{0};    ///< Index into material table ( 4 bytes)
            uint32_t  _pad0{0};         ///< Explicit padding          ( 4 bytes)
            uint32_t  _pad1{0};         ///< Explicit padding          ( 4 bytes)
        };                              //                             = 80 bytes

        /**
         * @brief Byte offsets and counts for one mesh within the flat scene geometry SSBOs.
         */
        struct MeshRange {
            uint32_t vertexOffset{0};  ///< First vertex index in the scene vertex SSBO
            uint32_t vertexCount{0};
            uint32_t indexOffset{0};   ///< First index in the scene index SSBO
            uint32_t indexCount{0};
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
         * @brief Push constants for the path-trace compute dispatch (128 bytes).
         */
        struct PathTracePushConstants {
            glm::vec3 cameraOrigin; float lensRadius;    // 16 B
            glm::vec3 lowerLeft;    float _p0{0};        // 16 B
            glm::vec3 horizontal;   float _p1{0};        // 16 B
            glm::vec3 vertical;     float _p2{0};        // 16 B
            glm::vec3 lensU;        float _p3{0};        // 16 B
            glm::vec3 lensV;        float _p4{0};        // 16 B
            uint32_t  frameIndex{0};
            uint32_t  samplesPerFrame{1};
            uint32_t  maxBounces{8};
            uint32_t  sphereCount{0};
            uint32_t  bvhNodeCount{0};
            uint32_t  imageW{0};
            uint32_t  imageH{0};
            uint32_t  _pad{0};                           // total 128 B
        };
        static_assert(sizeof(PathTracePushConstants) == 128);

        /**
         * @brief Push constants for the accumulate / tone-map composite dispatch.
         */
        struct AccumulatePushConstants {
            uint32_t totalSamples{1};
            uint32_t imageW{0};
            uint32_t imageH{0};
            uint32_t _pad{0};
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
            void     setMaterial(const Material &mat) override { m_cachedAlbedo = mat.getBaseColor(); }
            void     bindTexture(const std::string &, unsigned int, int) override {}
            void     setUniformMat4(const std::string &, const glm::mat4 &) override {}
            void     setUniformVec4(const std::string &, const glm::vec4 &) override {}
            void     setUniform1f(const std::string &, float) override {}
            void     setUniform1i(const std::string &, int) override {}
            void     setUniformVec3(const std::string &name, const glm::vec3 &v) override {
                if (name == "uAlbedo")     m_cachedAlbedo    = glm::vec4(v, m_cachedAlbedo.a);
                if (name == "uLightDir")   m_cachedLightDir  = v;
                if (name == "uLightColor") m_cachedLightColor = v;
                if (name == "uCameraPos")  m_cachedCameraPos  = v;
            }
            void     setDepthMask(bool) override {}
            void     setWireframe(bool) override {}
            void     initialiseShadowResources(int) override {}
            void     beginShadowPass() override {}
            void     endShadowPass() override {}
            void     drawMeshDepthOnly(const Mesh &, const glm::mat4 &) override {}
            void     beginDebugLines() override {}
            void     drawDebugLine(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &) override {}
            void     endDebugLines() override {}
            uint64_t getRenderTextureID()    const override { return reinterpret_cast<uint64_t>(m_offscreenDescSet); }
            uint64_t getShadowMapTextureID() const override { return 0; }
            bool     needsRenderTextureYFlip() const override { return false; }
            bool     supportsProceduralSky()   const override { return false; }

            // --- Path trace mode ---
            void setPathTraceMode(bool active) override { m_pathTraceMode = active; }

            // --- F-14: Scene geometry SSBOs ---
            void uploadSceneGeometry(const ResourceRegistry &registry) override;
            void updateInstanceBuffer(const RenderScene &scene) override;

            /** @brief Device-local flat vertex SSBO (all registered mesh vertices). */
            VkBuffer getSceneMeshVertexBuffer() const { return m_sceneVertexBuffer.buffer; }
            /** @brief Device-local flat index SSBO (all registered mesh indices). */
            VkBuffer getSceneMeshIndexBuffer()  const { return m_sceneIndexBuffer.buffer; }
            /** @brief Host-visible per-instance SSBO (updated every frame). */
            VkBuffer getInstanceBuffer()        const { return m_instanceBuffer.buffer; }
            /** @brief Number of instances written last frame. */
            uint32_t getInstanceCount()         const { return m_instanceCount; }
            /** @brief Range table: mesh name → vertex/index offsets inside the flat SSBOs. */
            const std::unordered_map<std::string, MeshRange> &getMeshRanges() const { return m_meshRanges; }

            // --- Path tracing ---

            /**
             * @brief Dispatch a path-trace frame and composite to the offscreen target.
             *
             * Performs the following work each call:
             *  1. Lazily creates the path-trace + accumulate compute pipelines and the
             *     RGBA32F accumulation image on the first invocation.
             *  2. Recreates the accumulation image if its dimensions no longer match
             *     @c m_offscreenExtent (i.e. after a viewport resize).
             *  3. Uploads @p spheres, @p materials, and @p bvhNodes to device-local
             *     SSBOs, rebuilding them every call.
             *  4. Updates all descriptor-set bindings (accum image, scene SSBOs,
             *     offscreen image) via @c updatePtDescriptorSets().
             *  5. Records a one-shot command buffer:
             *     - Optionally clears the accumulation image when @p resetAccum is true.
             *     - Dispatches @c path_trace.comp (16×16 workgroups).
             *     - Pipeline barrier: compute write → compute read on accum image.
             *     - Transitions offscreen image SHADER_READ_ONLY → GENERAL.
             *     - Dispatches @c accumulate.comp (tone-maps accum → offscreen).
             *     - Transitions offscreen image GENERAL → SHADER_READ_ONLY.
             *  6. Submits the command buffer to @c m_graphicsQueue and waits for idle.
             *
             * @note @p pc.imageW and @p pc.imageH are intentionally left at zero by
             *       PathTracingPass::render() and are injected here from
             *       @c m_offscreenExtent before the push-constant upload.  Both
             *       shaders use these values in a bounds-check guard; if either is
             *       zero every invocation returns immediately and nothing is written.
             *
             * @param spheres      Scene sphere primitives.
             * @param materials    Per-sphere material data.
             * @param bvhNodes     Flat BVH array produced by BvhBuilder::build().
             * @param pc           Camera + frame push-constant data.  @c imageW and
             *                     @c imageH are overwritten with @c m_offscreenExtent.
             * @param totalSamples Total accumulated sample count (for tone-map divide).
             * @param resetAccum   If true, clear the accumulation buffer before dispatch.
             */
            void dispatchPathTrace(const std::vector<GpuSphere>    &spheres,
                                   const std::vector<GpuMaterial>  &materials,
                                   const std::vector<GpuBvhNode>   &bvhNodes,
                                   const PathTracePushConstants     &pc,
                                   uint32_t                          totalSamples,
                                   bool                              resetAccum);

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
             * @brief Return the ImGui render pass (LOAD_OP_CLEAR, owns the swapchain image).
             */
            VkRenderPass getImGuiRenderPass() const { return m_imguiRenderPass; }

            /**
             * @brief Callbacks for ImGui texture registration (app layer sets these).
             *
             * VulkanBackend does not link ImGui directly. GUIRunner calls
             * setImGuiTextureCallbacks() after ImGui_ImplVulkan_Init(), then calls
             * registerOffscreenWithImGui() to allocate the descriptor set.
             */
            using ImGuiAddTextureFn    = std::function<VkDescriptorSet(VkSampler, VkImageView, VkImageLayout)>;
            using ImGuiRemoveTextureFn = std::function<void(VkDescriptorSet)>;
            void setImGuiTextureCallbacks(ImGuiAddTextureFn add, ImGuiRemoveTextureFn remove) {
                m_imguiAddTexture    = std::move(add);
                m_imguiRemoveTexture = std::move(remove);
            }

            /** @brief Allocate/re-allocate the offscreen ImGui descriptor set. */
            void registerOffscreenWithImGui();

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
            void createImGuiRenderPass();
            void createDepthResources();
            void createFramebuffers();
            void createSyncObjects();
            void createCommandBuffers();

            // --- Offscreen render target ---
            void createOffscreenResources();
            void destroyOffscreenResources();

            // --- F-08: Geometry pipeline ---
            void createGeometryPipeline();
            void createDescriptorPool();
            void createDescriptorSetLayout();
            void createDescriptorSets();
            void createUBOs();
            void uploadMesh(const Mesh &mesh);

            // --- F-14: Scene geometry helpers ---
            void destroySceneGeometryBuffers() noexcept;

            // --- Path-tracing helpers ---
            void createPathTracePipeline();
            void destroyPathTracePipeline() noexcept;
            void updatePtDescriptorSets();
            void createPtAccumImage(uint32_t w, uint32_t h);
            void destroyPtAccumImage() noexcept;

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
                             VkImage &image, VmaAllocation &alloc,
                             VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
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
            VkRenderPass m_imguiRenderPass{VK_NULL_HANDLE};

            // Framebuffers (one per swapchain image — ImGui pass only; geometry goes to offscreen FB)
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
            bool     m_frameStarted{false};    ///< true between a successful beginFrame and endFrame
            bool     m_pathTraceMode{false};   ///< when true, skip offscreen raster pass in beginFrame/endFrame

            // --- F-08: Geometry pipeline ---
            VkDescriptorSetLayout m_descSetLayout{VK_NULL_HANDLE};
            VkPipelineLayout      m_geometryPipelineLayout{VK_NULL_HANDLE};
            VkPipeline            m_geometryPipeline{VK_NULL_HANDLE};
            VkDescriptorPool      m_descriptorPool{VK_NULL_HANDLE};
            std::array<VkDescriptorSet, k_maxFramesInFlight> m_descriptorSets{};
            std::array<GpuBuffer,       k_maxFramesInFlight> m_uboBuffers{};
            std::array<void *,          k_maxFramesInFlight> m_uboMapped{};

            bool m_geometryPipelineReady{false};

            // Cached per-draw material / lighting state (flushed as push constants in drawMesh)
            glm::vec4 m_cachedAlbedo{0.8f, 0.8f, 0.8f, 1.0f};
            glm::vec3 m_cachedLightDir{0.4f, 1.0f, 0.6f};
            glm::vec3 m_cachedLightColor{1.0f, 1.0f, 1.0f};
            glm::vec3 m_cachedCameraPos{0.0f, 0.0f, 0.0f};

            // Mesh cache: meshName → GpuMesh
            std::unordered_map<std::string, GpuMesh> m_gpuMeshes;

            // Current frame view-projection
            ViewProjUBO m_viewProj{};

            // --- F-09: ImGui ---
            VulkanContextForImGui          m_imguiContext{};
            std::function<void(VkCommandBuffer)> m_imguiRenderCallback;
            ImGuiAddTextureFn              m_imguiAddTexture;
            ImGuiRemoveTextureFn           m_imguiRemoveTexture;

            // --- Offscreen render target (scene rendered here; displayed in ImGui viewport) ---
            // Resolve target (single-sample, SAMPLED — what ImGui displays)
            VkImage         m_offscreenColorImage{VK_NULL_HANDLE};
            VmaAllocation   m_offscreenColorAlloc{VK_NULL_HANDLE};
            VkImageView     m_offscreenColorView{VK_NULL_HANDLE};
            // MSAA color attachment (transient, not sampled)
            VkImage         m_msaaColorImage{VK_NULL_HANDLE};
            VmaAllocation   m_msaaColorAlloc{VK_NULL_HANDLE};
            VkImageView     m_msaaColorView{VK_NULL_HANDLE};
            // Depth (multisampled, matches m_msaaSamples)
            VkImage         m_offscreenDepthImage{VK_NULL_HANDLE};
            VmaAllocation   m_offscreenDepthAlloc{VK_NULL_HANDLE};
            VkImageView     m_offscreenDepthView{VK_NULL_HANDLE};

            VkSampler       m_offscreenSampler{VK_NULL_HANDLE};
            VkRenderPass    m_offscreenRenderPass{VK_NULL_HANDLE};
            VkFramebuffer   m_offscreenFramebuffer{VK_NULL_HANDLE};
            VkDescriptorSet m_offscreenDescSet{VK_NULL_HANDLE};  ///< ImGui texture handle
            VkExtent2D      m_offscreenExtent{};

            VkSampleCountFlagBits m_msaaSamples{VK_SAMPLE_COUNT_4_BIT}; ///< MSAA sample count

            // --- F-14: Flat scene geometry SSBOs ---
            GpuBuffer m_sceneVertexBuffer;                              ///< Device-local: all SceneVertex data
            GpuBuffer m_sceneIndexBuffer;                               ///< Device-local: all uint32 index data
            GpuBuffer m_instanceBuffer;                                 ///< Host-visible: per-frame GpuInstance array
            void     *m_instanceBufferMapped{nullptr};                  ///< Persistent map of m_instanceBuffer
            bool      m_sceneGeometryReady{false};

            std::unordered_map<std::string, MeshRange>  m_meshRanges;       ///< meshName → range in flat SSBOs
            std::unordered_map<std::string, uint32_t>   m_meshIndexMap;     ///< meshName → sequential uint32 index
            std::unordered_map<std::string, uint32_t>   m_materialIndexMap; ///< materialName → sequential uint32 ID

            static constexpr uint32_t k_maxInstances{4096};
            uint32_t m_instanceCount{0};

            // --- Path-tracing pipeline ---
            // Accumulation image (RGBA32F, storage — written each PT dispatch)
            VkImage         m_ptAccumImage{VK_NULL_HANDLE};
            VmaAllocation   m_ptAccumAlloc{VK_NULL_HANDLE};
            VkImageView     m_ptAccumView{VK_NULL_HANDLE};

            // Path-trace compute pipeline
            VkDescriptorSetLayout m_ptDescSetLayout{VK_NULL_HANDLE};
            VkDescriptorPool      m_ptDescPool{VK_NULL_HANDLE};
            VkDescriptorSet       m_ptDescSet{VK_NULL_HANDLE};
            VkPipelineLayout      m_ptPipelineLayout{VK_NULL_HANDLE};
            VkPipeline            m_ptPipeline{VK_NULL_HANDLE};

            // Accumulate / tone-map compute pipeline
            VkDescriptorSetLayout m_accumDescSetLayout{VK_NULL_HANDLE};
            VkDescriptorPool      m_accumDescPool{VK_NULL_HANDLE};
            VkDescriptorSet       m_accumDescSet{VK_NULL_HANDLE};
            VkPipelineLayout      m_accumPipelineLayout{VK_NULL_HANDLE};
            VkPipeline            m_accumPipeline{VK_NULL_HANDLE};

            // Scene SSBOs for path tracing (rebuilt on scene change)
            GpuBuffer m_ptSphereBuffer;
            GpuBuffer m_ptMaterialBuffer;
            GpuBuffer m_ptBvhBuffer;
            uint32_t  m_ptSphereCount{0};
            uint32_t  m_ptBvhNodeCount{0};
            bool      m_ptPipelineReady{false};

            /** @brief Dimensions the accumulation image was created at.
             *
             *  Compared against @c m_offscreenExtent each dispatch to detect when
             *  the viewport has been resized and the accum image must be reallocated.
             *  Reset to {0,0} by destroyPtAccumImage() so the next dispatch always
             *  allocates a fresh image.
             */
            VkExtent2D m_ptAccumExtent{0, 0};

            // Fixed offscreen colour format (R8G8B8A8_UNORM, supports STORAGE)
            static constexpr VkFormat k_offscreenColorFormat{VK_FORMAT_R8G8B8A8_UNORM};

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
