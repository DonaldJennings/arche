
#ifndef IMGUI_BACKEND_H
#define IMGUI_BACKEND_H

#include "IGUISystem.h"

struct GLFWwindow;

namespace Arche {
    namespace GUI {
        class IMGUIBackend final : public IGUISystem {
          public:
            explicit IMGUIBackend(GLFWwindow *window) : mainWindow{window} {};

            ~IMGUIBackend() override {
                if (isInitialised) {
                    Shutdown();
                }
            }

            void Startup() override;
            void Shutdown() override;
            void NewFrame() override;
            void Render() override;
            void SetDockController(std::function<void()> controller) override {
                dockController = controller;
            };

          private:
            GLFWwindow *mainWindow;
            std::function<void()> dockController;
            bool isInitialised = false;
        };

        std::unique_ptr<IGUISystem> CreateIMGUIBackend(GLFWwindow *window);
    } // namespace GUI
} // namespace Arche

#endif // IMGUI_BACKEND_H