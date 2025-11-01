#include "ImGuiBackend.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "ImGuiFontManager.h"

namespace Arche {
    namespace GUI {

        void IMGUIBackend::Startup() {
            IMGUI_CHECKVERSION();

            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();
            (void)io; // Avoid unused variable warning
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            setDarkTheme(true, 1.0f);

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGuiStyle &style = ImGui::GetStyle();
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

            ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
            ImGui_ImplOpenGL3_Init("#version 330");

            // Rebuild fonts at the platform DPI so text renders crisply on high-DPI displays
            float platformScale = GetPlatformDpiScale();
            if (!(platformScale > 0.0f)) {
                // fallback to ImGui framebuffer scale if platform query failed
                platformScale = io.DisplayFramebufferScale.x;
            }
            if (!(platformScale > 0.0f)) platformScale = 1.0f;

            RebuildImGuiFontsForDpi(platformScale);

            isInitialised = true;
        }

        void IMGUIBackend::Shutdown() {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            isInitialised = false;
        }

        void IMGUIBackend::NewFrame() {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (dockController) {
                dockController();
            }
        }

        void IMGUIBackend::Render() { 
            ImGui::Render(); 
            int displayW, displayH;

            glfwGetFramebufferSize(mainWindow, &displayW, &displayH);
            glViewport(0, 0, displayW, displayH);
            glClearColor(0.10f, 0.12f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }
        }

        std::unique_ptr<IGUISystem> CreateIMGUIBackend(GLFWwindow* mainWindow) {
			return std::make_unique<IMGUIBackend>(mainWindow);
		}

    } // namespace GUI
} // namespace Arche