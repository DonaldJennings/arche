#ifndef IMGUI_BACKEND_H
#define IMGUI_BACKEND_H

#include "IGUISystem.h"
#include "Theme.h"

#include <string>

struct GLFWwindow;

#ifdef ARCHE_BACKEND_VULKAN
#include <vulkan/vulkan.h>
// Forward-declare VulkanContextForImGui to avoid pulling in the full VulkanBackend header here.
namespace Arche { namespace Render { struct VulkanContextForImGui; } }
#endif

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
            void RunSimulation()   { simulationState = SimulationState::RUNNING; };
            bool IsSimulationRunning() const { return simulationState == SimulationState::RUNNING; };

            void SetDockController(std::function<void()> controller) override {
                dockController = controller;
            };

            // Branding helpers
            void LoadAndSetWindowIcon(const std::string &path);
            void DrawLogoInMenuBar();

#ifdef ARCHE_BACKEND_VULKAN
            /**
             * @brief Provide the Vulkan context needed to initialise the ImGui Vulkan backend.
             *
             * Must be called before Startup(). The pointer must remain valid for the
             * lifetime of this object (owned by VulkanBackend / EngineCore).
             */
            void setVulkanContext(const Arche::Render::VulkanContextForImGui *ctx) {
                m_vulkanContext = ctx;
            }
#endif

          private:
            GLFWwindow *mainWindow;
            std::function<void()> dockController;
            bool isInitialised = false;

#ifdef ARCHE_BACKEND_VULKAN
            const Arche::Render::VulkanContextForImGui *m_vulkanContext{nullptr};
            VkDescriptorPool m_imguiDescriptorPool{VK_NULL_HANDLE};
            VkDevice         m_vkDevice{VK_NULL_HANDLE};
#endif
        };

        std::unique_ptr<IGUISystem> CreateIMGUIBackend(GLFWwindow *window);
    } // namespace GUI
} // namespace Arche

#endif // IMGUI_BACKEND_H
