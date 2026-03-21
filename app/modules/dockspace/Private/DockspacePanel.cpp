#include "DockspacePanel.h"

#include <Theme.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <memory>

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

        DockspacePanel::DockspacePanel(std::shared_ptr<EditorSession> contextIn)
            : name{"Dockspace"}, context(std::move(contextIn)) {}

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

            // Removed ImGuiWindowFlags_NoDocking so other windows can dock into this main window
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            ImGui::Begin(name.c_str(), nullptr, window_flags);

            int lw, lh;
            auto exeDir = GetExecutableDir();
            auto logoPath = (exeDir / "assets" / "logo" / "arche-logo.png").string();
            // unsigned int tex = LoadLogoTexture(logoPath, lw, lh);
            // if (tex) {
            //     ImGui::Image((void *)(intptr_t)tex, ImVec2(64.0f * dpiScale, 64.0f* dpiScale));
            //     ImGui::SameLine();
            // }

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
            ImGui::PopStyleVar(2);

            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

            // Build the default layout once — only when no dockspace node exists yet
            if (!ImGui::DockBuilderGetNode(dockspace_id)) {
                ImGui::DockBuilderRemoveNode(dockspace_id);
                ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

                // Split off the bottom strip for output panels
                ImGuiID dock_upper, dock_bottom;
                ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, &dock_bottom, &dock_upper);

                // Split the upper area into left sidebar, center viewport, right sidebar
                ImGuiID dock_left, dock_center_right;
                ImGui::DockBuilderSplitNode(dock_upper, ImGuiDir_Left, 0.20f, &dock_left, &dock_center_right);

                ImGuiID dock_center, dock_right;
                ImGui::DockBuilderSplitNode(dock_center_right, ImGuiDir_Right, 0.25f, &dock_right, &dock_center);

                // Assign panels to regions
                ImGui::DockBuilderDockWindow("World Properties",  dock_left);
                ImGui::DockBuilderDockWindow("3DViewport",        dock_center);
                ImGui::DockBuilderDockWindow("Entity Inspector",  dock_right);

                // Stack all output panels as tabs in the bottom strip
                ImGui::DockBuilderDockWindow("Log",               dock_bottom);
                ImGui::DockBuilderDockWindow("Metrics",           dock_bottom);
                ImGui::DockBuilderDockWindow("Material Browser",  dock_bottom);
                ImGui::DockBuilderDockWindow("Shader Browser",    dock_bottom);

                ImGui::DockBuilderFinish(dockspace_id);
            }

            ImGui::End();
        }

        std::string_view DockspacePanel::GetName() const { return name; }

    } // namespace GUI
} // namespace Arche
