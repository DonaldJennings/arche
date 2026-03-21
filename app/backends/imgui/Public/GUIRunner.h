// gui/backends/imgui/Public/GUIRunner.h
#pragma once
#include <memory>
#include "IGUISystem.h"
#include "GLFWWindow_RAII.h"

#ifdef ARCHE_BACKEND_VULKAN
namespace Arche { namespace Render {
    struct VulkanContextForImGui;
    class VulkanBackend;
} }
#endif

namespace Arche {
namespace GUI {

class GUIRunner {
public:
    explicit GUIRunner(std::shared_ptr<GLFWWindowHandle> window);

#ifdef ARCHE_BACKEND_VULKAN
    /**
     * @brief Construct with a VulkanBackend so that ImGui can be wired into the Vulkan
     *        frame loop.  The backend pointer must outlive this GUIRunner.
     */
    GUIRunner(std::shared_ptr<GLFWWindowHandle> window,
              Arche::Render::VulkanBackend *vulkanBackend);
#endif

    ~GUIRunner();

    GUIRunner(const GUIRunner&) = delete;
    GUIRunner& operator=(const GUIRunner&) = delete;
    GUIRunner(GUIRunner&&) = delete;
    GUIRunner& operator=(GUIRunner&&) = delete;

    template <class Func>
    void setDockController(Func&& func) { guiSystem->SetDockController(std::forward<Func>(func)); }

    void frame();

private:
    std::unique_ptr<IGUISystem> guiSystem;
};

} // namespace GUI
} // namespace Arche
