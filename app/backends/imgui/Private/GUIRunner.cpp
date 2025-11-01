#include "GUIRunner.h"
#include "ImGuiBackend.h"

namespace Arche {
namespace GUI {

GUIRunner::GUIRunner(std::shared_ptr<GLFWWindowHandle> glfwWindow) {
    guiSystem = CreateIMGUIBackend(*glfwWindow);
    guiSystem->Startup();
}

GUIRunner::~GUIRunner() {
    if (guiSystem) {
        guiSystem->Shutdown();
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