#include "MetricsPanel.h"

#include <imgui.h>
#include <WorldSystem.h>
#include <Vector3D.h>
#include <World.h>

#include <iostream>

namespace Arche {
    namespace GUI {

        MetricsPanel::MetricsPanel(std::shared_ptr<UIContext> contextIn)
            : name{"Metrics"}, context(std::move(contextIn)) {}

        void MetricsPanel::Draw() {
            ImGui::Begin(name.c_str());
            // Show frame rate and frame time
            ImGuiIO &io = ImGui::GetIO();
            ImGui::Text("Framerate: %.1f FPS", io.Framerate);
            ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);

            if (context && context->worldSystem && context->worldSystem->getWorld()) {
                auto world = context->worldSystem->getWorld();
                ImGui::Text("World Simulation Time: %.2f s", world->simulationTime());

                auto view{world->view()};

                Arche::Math::Vector3D gravity{world->gravity()};

                ImGui::SeparatorText("World Properties");   
                ImGui::Text("Gravitational Force: (%.1f, %.1f, %.1f)", gravity.x(), gravity.y(), gravity.z());
                ImGui::Text("Gravity Magnitude: %.2f", gravity.length());

                if (ImGui::BeginTable("Bodies", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Index");
                    ImGui::TableSetupColumn("Position");
                    ImGui::TableSetupColumn("Velocity");
                    ImGui::TableSetupColumn("Mass");
                    ImGui::TableHeadersRow();

                    for (size_t i = 0; i < view.bodies.size(); ++i) {
                        const auto &body = view.bodies[i];
                        const auto &pos = body.transform.getPosition();
                        const auto &vel = body.velocity;
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%zu", i);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("(%.2f, %.2f, %.2f)", pos.x(), pos.y(), pos.z());
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("(%.2f, %.2f, %.2f)", vel.x(), vel.y(), vel.z());
                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("%.2f", body.mass);
                    }
                    ImGui::EndTable();
                }
            } else {
                ImGui::TextUnformatted("No world data available");
            }
            ImGui::End();
        }

        std::string_view MetricsPanel::GetName() const { return name; }

    } // namespace GUI
} // namespace Arche
