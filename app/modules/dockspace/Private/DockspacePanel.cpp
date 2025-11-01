#include "DockspacePanel.h"

#include <Theme.h>
#include <imgui.h>

#include <memory>

#include <Vector3D.h>
#include <World.h>
#include <cmath> // for fabsf

#include "ImageManager.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <filesystem>

static std::filesystem::path GetExecutableDir() {
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH);
    if (len == 0)
        return std::filesystem::current_path();
    return std::filesystem::path(buf).parent_path();
#else
    char buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = 0;
        return std::filesystem::path(buf).parent_path();
    }
    return std::filesystem::current_path();
#endif
}

namespace Arche {
    namespace GUI {

        DockspacePanel::DockspacePanel(std::shared_ptr<UIContext> contextIn) : name{"Dockspace"}, context(std::move(contextIn)) {}

        void DockspacePanel::Draw() {
            ImGuiIO &io = ImGui::GetIO();

            // Attempt to get framebuffer scale first, but fall back to native DPI on Windows
            float dpiScale = io.DisplayFramebufferScale.x;
            if (!(dpiScale > 0.0f))
                dpiScale = 1.0f;

#ifdef _WIN32
            // If framebuffer scale is effectively 1.0 (backend didn't set it), query Win32 DPI
            if (dpiScale <= 1.01f) {
                ImGuiViewport *vp = ImGui::GetMainViewport();
                UINT dpi = 0;
                if (vp && vp->PlatformHandle) {
                    HWND hwnd = (HWND)vp->PlatformHandle;
                    // GetDpiForWindow is available on Windows 10 (1607) and later.
                    dpi = ::GetDpiForWindow(hwnd);
                }
                if (dpi == 0) {
                    // fallback to system DPI
                    dpi = ::GetDpiForSystem();
                }
                if (dpi > 0) {
                    dpiScale = static_cast<float>(dpi) / 96.0f;
                }
            }
#endif

            // Apply DPI-aware scaling to fonts and style once (or when DPI changes),
            // using a saved original style to avoid compounding ScaleAllSizes calls.
            static bool s_initialized = false;
            static ImGuiStyle s_originalStyle;
            static float s_baseFontScale = 1.0f;
            static float s_appliedDpiScale = 0.0f;

            if (!s_initialized) {
                s_originalStyle = ImGui::GetStyle();  // save the original style
                s_baseFontScale = io.FontGlobalScale; // save initial font scale (usually 1.0f)
                s_initialized = true;
            }

            if (std::fabs(s_appliedDpiScale - dpiScale) > 0.001f) {
                // Reset style to original, then scale from that baseline.
                ImGui::GetStyle() = s_originalStyle;
                ImGui::GetStyle().ScaleAllSizes(dpiScale);

                // Apply font global scale relative to saved base.
                io.FontGlobalScale = s_baseFontScale * dpiScale;

                s_appliedDpiScale = dpiScale;
            }

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            ImGui::Begin(name.c_str(), nullptr, window_flags);

                            // Draw logo on left
            int lw, lh;
            auto exeDir = GetExecutableDir();
            auto logoPath = (exeDir / "assets" / "logo" / "arche-logo.png").string();
            unsigned int tex = LoadLogoTexture(logoPath, lw, lh);
            if (tex) {
                ImGui::Image((void *)(intptr_t)tex, ImVec2(64.0f * dpiScale, 64.0f* dpiScale));
                ImGui::SameLine();
            }

            // Menu bar: only simple placeholders for Save/Load and a Pre-defined demos submenu.
            if (ImGui::BeginMenuBar()) {

                if (ImGui::BeginMenu("File")) {
                    // Placeholder: Save state (no-op for now)
                    if (ImGui::MenuItem("Save state")) {
                    }

                    // Load state submenu (placeholder)
                    if (ImGui::BeginMenu("Load state")) {
                        // future load items go here
                        ImGui::EndMenu();
                    }

                    // Pre-defined demos: keep sample demos here
                    if (ImGui::BeginMenu("Pre-defined demos")) {
                        if (ImGui::MenuItem("Earth Gravity")) {
                            Arche::Core::WorldConfig config;
                            config.gravity = Arche::Math::Vector3D(0.0f, 9.81f, 0.0f);
                            config.stepDuration = 1.0f / 60.0f;
                            auto newWorld = Arche::Scene::World::Create(config);
                            context->worldSystem->setWorld(newWorld);
                            RunParticlesDemo(newWorld);
                        }

                        if (ImGui::MenuItem("Moon Gravity")) {
                            Arche::Core::WorldConfig config;
                            config.gravity = Arche::Math::Vector3D(0.0f, 1.62f, 0.0f);
                            config.stepDuration = 1.0f / 60.0f;
                            auto newWorld = Arche::Scene::World::Create(config);
                            context->worldSystem->setWorld(newWorld);
                            RunParticlesDemo(newWorld);
                        }

                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            ImGui::PopStyleVar(2);

            // --- Simulation control toolbar (centered large buttons) ---
            ImGui::Spacing();
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

            const float childHeight = 60.0f * dpiScale;
            ImGui::BeginChild("SimulationToolbar", ImVec2(0, childHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration);

            ImVec2 btnSize(140.0f * dpiScale, 42.0f * dpiScale);

            // Center the buttons horizontally and vertically in the child
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float availW = avail.x;
            ImGuiStyle &style = ImGui::GetStyle();
            const int btnCount = 4;
            float spacing = style.ItemSpacing.x;
            float totalButtonsW = btnCount * btnSize.x + (btnCount - 1) * spacing;
            float startX = (availW - totalButtonsW) * 0.5f;
            if (startX < 0.0f)
                startX = 0.0f;
            ImGui::SetCursorPosX(startX);

            float startY = (childHeight - btnSize.y) * 0.5f;
            if (startY > 0.0f)
                ImGui::SetCursorPosY(startY);

            // Large buttons for Play, Pause, Step, Reset (centered)
            if (ImGui::Button("Play", btnSize)) {
                if (context)
                    context->runSimulation();
            }
            ImGui::SameLine();
            if (ImGui::Button("Pause", btnSize)) {
                if (context)
                    context->pauseSimulation();
            }
            ImGui::SameLine();
            if (ImGui::Button("Step", btnSize)) {
                if (context && context->worldSystem && context->worldSystem->getWorld()) {
                    auto w = context->worldSystem->getWorld();
                    w->step(w->stepDuration());
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset", btnSize)) {
                if (context) {
                    Arche::Core::WorldConfig config;
                    config.gravity = Arche::Math::Vector3D(0.0f, 98.1f, 0.0f);
                    config.stepDuration = 1.0f / 60.0f;
                    config.maxSubSteps = 5;
                    config.deterministic = true;
                    auto newWorld = Arche::Scene::World::Create(config);
                    context->worldSystem->setWorld(newWorld);
                }
            }

            ImGui::EndChild();
            ImGui::PopStyleVar(2);

            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

            ImGui::End();
        }

        std::string_view DockspacePanel::GetName() const { return name; }

        void DockspacePanel::RunParticlesDemo(std::shared_ptr<Arche::Scene::World> world) {
            float y = 10.0f;
            float xStart = 20.0f;
            float xEnd = 380.0f;
            float step = (xEnd - xStart) / 9.0f;
            for (int i = 0; i < 10; ++i) {
                float x = xStart + i * step;
                Arche::Math::SpatialTransform transform;
                transform.setPosition(Arche::Math::Vector3D(x, 0.0f, 0.0f));
                world->createParticle(transform, 1.0f);
            }
        }

    } // namespace GUI
} // namespace Arche
