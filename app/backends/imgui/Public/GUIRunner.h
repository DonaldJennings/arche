// gui/backends/imgui/Public/GUIRunner.h
#pragma once
#include <memory>
#include "IGUISystem.h"
#include "GLFWWindow_RAII.h"

namespace Arche {
namespace GUI {

class GUIRunner {
public:
    explicit GUIRunner(std::shared_ptr<GLFWWindowHandle> window);
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