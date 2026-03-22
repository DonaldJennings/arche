#include "GUIRunner.h"
#include "ImGuiBackend.h"

#ifdef ARCHE_BACKEND_VULKAN
#include "VulkanBackend.h"
#include <backends/imgui_impl_vulkan.h>
#endif

namespace Arche {
namespace GUI {

GUIRunner::GUIRunner(std::shared_ptr<GLFWWindowHandle> glfwWindow) {
    guiSystem = CreateIMGUIBackend(*glfwWindow);
    guiSystem->Startup();
}

#ifdef ARCHE_BACKEND_VULKAN
GUIRunner::GUIRunner(std::shared_ptr<GLFWWindowHandle> glfwWindow,
                     Arche::Render::VulkanBackend *vulkanBackend)
{
    auto *imguiBackend = new IMGUIBackend(*glfwWindow);

    // Provide the Vulkan context before Startup
    if (vulkanBackend) {
        imguiBackend->setVulkanContext(vulkanBackend->getVulkanContextForImGui());
    }

    guiSystem.reset(imguiBackend);
    guiSystem->Startup();

    // Wire up ImGui texture callbacks (engine renderer doesn't link ImGui directly)
    // and register the offscreen render target now that ImGui_ImplVulkan_Init() ran.
    if (vulkanBackend) {
        vulkanBackend->setImGuiTextureCallbacks(
            [](VkSampler s, VkImageView v, VkImageLayout l) {
                return ImGui_ImplVulkan_AddTexture(s, v, l);
            },
            [](VkDescriptorSet d) {
                ImGui_ImplVulkan_RemoveTexture(d);
            });
        vulkanBackend->registerOffscreenWithImGui();
    }

    // Register the render callback. It fires inside VulkanBackend::endFrame() which
    // is called via RenderingSystem::present() — after ImGui::Render() — so draw
    // data is guaranteed to be valid at that point.
    if (vulkanBackend) {
        vulkanBackend->setImGuiRenderCallback([](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
        });
    }
}
#endif

GUIRunner::~GUIRunner() {
    shutdown();
}

void GUIRunner::shutdown() {
    if (guiSystem) {
        guiSystem->Shutdown();
        guiSystem.reset();
    }
}

void GUIRunner::frame() {
    if (guiSystem) {
        guiSystem->NewFrame();
        guiSystem->Render();
    }
}

} // namespace GUI
} // namespace Arche
