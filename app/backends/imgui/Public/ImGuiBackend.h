#ifndef IMGUI_BACKEND_H
#define IMGUI_BACKEND_H

#include "IGUISystem.h"
#include "Theme.h"

#include <string>
struct GLFWwindow;

namespace Arche {
    namespace GUI {

        enum class SimulationState { RUNNING, PAUSED };

        class IMGUIBackend final : public IGUISystem {
          private:
            SimulationState simulationState = SimulationState::PAUSED;

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

            void PauseSimulation() { simulationState = SimulationState::PAUSED; };

            void RunSimulation() { simulationState = SimulationState::RUNNING; };

            bool IsSimulationRunning() const { return simulationState == SimulationState::RUNNING; };

            void SetDockController(std::function<void()> controller) override { dockController = controller; };

            // Branding helpers
            void LoadAndSetWindowIcon(const std::string& path);
            void DrawLogoInMenuBar();

          private:
            GLFWwindow *mainWindow;
            std::function<void()> dockController;
            bool isInitialised = false;
        };

        std::unique_ptr<IGUISystem> CreateIMGUIBackend(GLFWwindow *window);
    } // namespace GUI
} // namespace Arche

#endif // IMGUI_BACKEND_H