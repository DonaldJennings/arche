
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "DockspacePanel.h"
#include "GUILogSink.h"
#include "ImGuiBackend.h"
#include "LogPanel.h"
#include "MetricsPanel.h"
#include "PanelFactory.h"
#include "Viewport2DPanel.h"

#include "GlobalLogger.h"

namespace Arche {
    namespace Application {
        struct GLFWInitialiser {
            GLFWInitialiser() {
                glfwSetErrorCallback([](int error, const char *description) { fprintf(stderr, "GLFW Error %d: %s\n", error, description); });

                if (!glfwInit()) {
                    throw std::runtime_error("Failed to initialize GLFW");
                }
            }

            ~GLFWInitialiser() { glfwTerminate(); }

            GLFWInitialiser(const GLFWInitialiser &) = delete;
            GLFWInitialiser &operator=(const GLFWInitialiser &) = delete;
            GLFWInitialiser(GLFWInitialiser &&) = delete;
            GLFWInitialiser &operator=(GLFWInitialiser &&) = delete;
        };

        struct GLFWWindow_RAII {
            GLFWwindow *window;

            GLFWWindow_RAII(int width, int height, const char *title) {
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
                window = glfwCreateWindow(width, height, title, nullptr, nullptr);
                if (!window) {
                    throw std::runtime_error("Failed to create GLFW window");
                }
                glfwMakeContextCurrent(window);
                glfwSwapInterval(1); // Enable vsync
            }

            ~GLFWWindow_RAII() {
                if (window) {
                    glfwDestroyWindow(window);
                }
            }

            GLFWWindow_RAII(const GLFWWindow_RAII &) = delete;
            GLFWWindow_RAII &operator=(const GLFWWindow_RAII &) = delete;
            GLFWWindow_RAII(GLFWWindow_RAII &&) = delete;
            GLFWWindow_RAII &operator=(GLFWWindow_RAII &&) = delete;

            operator GLFWwindow *() { return window; }
        };

        struct GUIRunner {
            std::unique_ptr<GUI::IGUISystem> guiSystem;

            explicit GUIRunner(std::shared_ptr<GLFWWindow_RAII> glfwWindow) : guiSystem(GUI::CreateIMGUIBackend(*glfwWindow)) { guiSystem->Startup(); }

            ~GUIRunner() { guiSystem->Shutdown(); }

            GUIRunner(const GUIRunner &) = delete;
            GUIRunner &operator=(const GUIRunner &) = delete;
            GUIRunner(GUIRunner &&) = delete;
            GUIRunner &operator=(GUIRunner &&) = delete;

            template <class Func> void setDockController(Func &&func) { guiSystem->SetDockController(std::forward<Func>(func)); }

            void frame() {
                guiSystem->NewFrame();
                guiSystem->Render();
            }
        };
    } // namespace Application
} // namespace Arche

int main() {
    try {

        auto guiLogger{std::make_unique<Arche::GUI::GUILogSink>()};
        auto *guiLoggerPtr{guiLogger.get()};

        Arche::Core::GetGlobalLogger()->addSink(std::move(guiLogger));

        Arche::Application::GLFWInitialiser glfwInitialiser;
        std::shared_ptr<Arche::Application::GLFWWindow_RAII> window{std::make_shared<Arche::Application::GLFWWindow_RAII>(1280, 720, "Arche Engine")};
        Arche::Application::GUIRunner guiRunner(window);

        Arche::GUI::PanelRegistry panels;

        Arche::Core::WorldConfig config;

        config.stepDuration = 1.0f / 60.0f;
        config.gravity = Arche::Math::Vector3D(0.0f, 98.1f, 0.0f); // Gravity pointing downwards

        panels.RegisterPanel(std::make_shared<Arche::GUI::DockspacePanel>());
        panels.RegisterPanel(std::make_shared<Arche::GUI::LogPanel>(guiLoggerPtr));
        panels.RegisterPanel(std::make_shared<Arche::GUI::MetricsPanel>());

        auto simulatedWorld{Arche::Scene::World::Create(config)};

        static auto guiLogSink{std::make_shared<Arche::GUI::GUILogSink>()};

        panels.RegisterPanel(std::make_shared<Arche::GUI::Viewport2DPanel>(std::move(simulatedWorld)));


        guiRunner.setDockController([&]() {
            panels.DrawPanels();
        });

        while (!glfwWindowShouldClose(*window)) {
            glfwPollEvents();

            guiRunner.frame();

            glfwSwapBuffers(*window);
        }
    } catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }
}