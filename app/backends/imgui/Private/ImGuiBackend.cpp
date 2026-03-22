#include "ImGuiBackend.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#ifndef ARCHE_BACKEND_VULKAN
// ---- OpenGL path ----
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#else
// ---- Vulkan path ----
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "VulkanBackend.h"
#endif

#include "ImageManager.h"
#include "ImGuiFontManager.h"
#include <stb_image.h>

namespace Arche {
    namespace GUI {

        void IMGUIBackend::Startup() {
            IMGUI_CHECKVERSION();

            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            setDarkTheme(true, 1.0f);

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGuiStyle &style = ImGui::GetStyle();
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

#ifndef ARCHE_BACKEND_VULKAN
            // ---- OpenGL initialisation ----
            ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
            ImGui_ImplOpenGL3_Init("#version 330");
#else
            // ---- Vulkan initialisation ----
            const Arche::Render::VulkanContextForImGui *vkCtx = m_vulkanContext;
            if (!vkCtx) {
                throw std::runtime_error("IMGUIBackend: Vulkan context not set before Startup()");
            }

            ImGui_ImplGlfw_InitForVulkan(mainWindow, true);

            // Create a dedicated descriptor pool for ImGui
            VkDescriptorPoolSize poolSize{};
            poolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSize.descriptorCount = 100;

            VkDescriptorPoolCreateInfo poolCI{};
            poolCI.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCI.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            poolCI.maxSets       = 100;
            poolCI.poolSizeCount = 1;
            poolCI.pPoolSizes    = &poolSize;

            if (vkCreateDescriptorPool(vkCtx->device, &poolCI, nullptr,
                                       &m_imguiDescriptorPool) != VK_SUCCESS)
                throw std::runtime_error("IMGUIBackend: failed to create ImGui descriptor pool");

            // Newer Dear ImGui versions (2025+) use PipelineInfoMain.RenderPass
            ImGui_ImplVulkan_InitInfo initInfo{};
            initInfo.ApiVersion      = VK_API_VERSION_1_2;
            initInfo.Instance        = vkCtx->instance;
            initInfo.PhysicalDevice  = vkCtx->physicalDevice;
            initInfo.Device          = vkCtx->device;
            initInfo.QueueFamily     = vkCtx->graphicsQueueFamily;
            initInfo.Queue           = vkCtx->graphicsQueue;
            initInfo.DescriptorPool  = m_imguiDescriptorPool;
            initInfo.MinImageCount   = vkCtx->minImageCount;
            initInfo.ImageCount      = vkCtx->imageCount;
            // Set render pass via the new PipelineInfoMain structure
            initInfo.PipelineInfoMain.RenderPass      = vkCtx->imguiRenderPass;
            initInfo.PipelineInfoMain.MSAASamples      = VK_SAMPLE_COUNT_1_BIT;

            if (!ImGui_ImplVulkan_Init(&initInfo))
                throw std::runtime_error("IMGUIBackend: ImGui_ImplVulkan_Init failed");

            // ImGui 1.92+ sets RendererHasViewports unconditionally even when
            // ViewportsEnable is off. Per-window Vulkan swapchains are not yet
            // implemented, so clear the flag to prevent null viewport callbacks.
            ImGui::GetIO().BackendFlags &= ~ImGuiBackendFlags_RendererHasViewports;

            // Store device for shutdown
            m_vkDevice = vkCtx->device;
#endif

            // set window icon (best effort)
            if (mainWindow) {
                SetWindowIconFromFile(mainWindow, "assets/logo/arche-logo.png");
            }

            float platformScale = GetPlatformDpiScale();
            if (!(platformScale > 0.0f)) {
                platformScale = io.DisplayFramebufferScale.x;
            }
            if (!(platformScale > 0.0f)) platformScale = 1.0f;

            RebuildImGuiFontsForDpi(platformScale);

            isInitialised = true;
        }

        void IMGUIBackend::Shutdown() {
#ifndef ARCHE_BACKEND_VULKAN
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
#else
            if (m_vkDevice != VK_NULL_HANDLE)
                vkDeviceWaitIdle(m_vkDevice);

            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();

            if (m_imguiDescriptorPool != VK_NULL_HANDLE && m_vkDevice != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(m_vkDevice, m_imguiDescriptorPool, nullptr);
                m_imguiDescriptorPool = VK_NULL_HANDLE;
            }
#endif
            ImGui::DestroyContext();
            isInitialised = false;
        }

        void IMGUIBackend::NewFrame() {
#ifndef ARCHE_BACKEND_VULKAN
            ImGui_ImplOpenGL3_NewFrame();
#else
            ImGui_ImplVulkan_NewFrame();
#endif
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (dockController) {
                dockController();
            }
        }

        void IMGUIBackend::Render() {
            ImGui::Render();

#ifndef ARCHE_BACKEND_VULKAN
            // OpenGL: clear + blit
            int displayW, displayH;
            glfwGetFramebufferSize(mainWindow, &displayW, &displayH);
            glViewport(0, 0, displayW, displayH);
            glClearColor(0.10f, 0.12f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow *backup = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup);
            }
#else
            // Vulkan: ImGui draw data is submitted via the render callback registered with
            // VulkanBackend in GUIRunner. Nothing else to do here except handle viewports.
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
#endif
        }

        std::unique_ptr<IGUISystem> CreateIMGUIBackend(GLFWwindow *mainWindow) {
            return std::make_unique<IMGUIBackend>(mainWindow);
        }

    } // namespace GUI
} // namespace Arche
