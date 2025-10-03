#pragma once

#include <IPanel.h>
#include <imgui.h>
#include <string_view>

namespace Arche {
    namespace GUI {
        class DockspacePanel : public IPanel {
          public:
            DockspacePanel() : name{"Dockspace"} {}

            void Draw() override {
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

                ImGui::PopStyleVar(2);

                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

                ImGui::End();
            }

            std::string_view GetName() const override { return name; }

          private:
            std::string name;
        };
    } // namespace GUI
} // namespace Arche