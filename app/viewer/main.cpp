
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "ImGuiBackend.h"

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

void MainWindow() {
    ImGui::Begin("Hello, world!");
    ImGui::TextUnformatted("GUI is running...");
    ImGui::Separator();
    ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);
    ImGui::End();
}
int main() {
    try {
        Arche::Application::GLFWInitialiser glfwInitialiser;
        std::shared_ptr<Arche::Application::GLFWWindow_RAII> window{std::make_shared<Arche::Application::GLFWWindow_RAII>(1280, 720, "Arche Engine")};
        Arche::Application::GUIRunner guiRunner(window);

        guiRunner.setDockController([]() { MainWindow(); });

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
