#pragma once

#include <imgui.h>
#include <string>

#include <IPanel.h>

namespace Arche {
    namespace GUI {
        class MetricsPanel : public IPanel {
          public:
            MetricsPanel() : name{"Metrics"} {}

            void Draw() override {
                ImGui::Begin(name.c_str());
                // Show frame rate and frame time
                ImGuiIO &io = ImGui::GetIO();
                ImGui::Text("Framerate: %.1f FPS", io.Framerate);
                ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);

                // Optionally show ImGui's built-in metrics window
                if (ImGui::CollapsingHeader("ImGui Metrics")) {
                    ImGui::ShowMetricsWindow(nullptr);
                }

                ImGui::End();
            }

            inline std::string_view GetName() const override { return name; }

          private:
            std::string name;
        };

    } // namespace GUI
} // namespace Arche