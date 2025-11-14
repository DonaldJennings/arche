#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "WorldProperties.h"

#include <WorldSystem.h>
#include <imgui.h>

#include <algorithm> // for std::max, std::min
#include <array>
#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

namespace Arche {
    namespace GUI {

        void WorldProperties::Draw() {
            ImGui::Begin(name.c_str());

            auto &globalSettings = context->globalSettings();
            auto &renderSettings = globalSettings.getRenderSettings();
            auto &worldSettings = globalSettings.getWorldSettings();

            if (ImGui::CollapsingHeader("Renderer Configuration")) {
                ImGui::ColorEdit4("Clear Color", (float *)(&renderSettings.clearColor));
                ImGui::SliderFloat("Field of View", &renderSettings.fieldOfView, 30.0f, 120.0f);
                ImGui::Checkbox("VSync", &renderSettings.vsync);
            }

            if (ImGui::CollapsingHeader("Editor Settings")) {

                ImGui::Checkbox("Wireframe Mode", &renderSettings.wireframe);
                ImGui::Checkbox("Show Grid", &renderSettings.showGrid);
                ImGui::Checkbox("Show Axes", &renderSettings.showAxes);
            }

            if (ImGui::CollapsingHeader("Lighting")) {
                ImGui::ColorEdit3("Ambient Light", &renderSettings.ambientLight.x);
                ImGui::SliderFloat("Ambient Strength", &renderSettings.ambientStrength, 0.0f, 1.0f);
            }

            if (ImGui::CollapsingHeader("World Configuration")) {
                ImGui::DragFloat3("Gravity", &worldSettings.gravity.x, 0.1f, -100.0f, 100.0f);
                ImGui::SliderFloat("Time Scale", &worldSettings.timeScale, 0.0f, 5.0f);
            }

            if (ImGui::CollapsingHeader("Main Camera Configuration")) {
            }
            // Existing world info
            if (context && context->worldSystem()) {
                auto world = context->worldSystem();

                auto view{world->view()};

                ImGui::SeparatorText("Physics Properties");
                ImGui::Text("Number of Bodies: %zu", view.bodies.size());
                ImGui::Text("Simulation State: %s", context->simulationIsPaused() ? "Paused" : "Running");

                ImGui::SeparatorText("World Properties");

                static std::uint64_t selectedEntityId = 0;

                ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
                                        ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_RowBg |
                                        ImGuiTableFlags_Borders;

                if (ImGui::BeginTable("Bodies", 10, flags)) {
                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Pos");
                    ImGui::TableSetupColumn("Rot");
                    ImGui::TableSetupColumn("Scale");
                    ImGui::TableSetupColumn("Speed");
                    ImGui::TableSetupColumn("Mass");
                    ImGui::TableSetupColumn("Mesh");
                    ImGui::TableSetupColumn("Material");
                    ImGui::TableSetupColumn("Shader");

                    ImGui::TableHeadersRow();

                    for (auto &e : view.bodies) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);

                        bool selected = (selectedEntityId == e->getID());
                        if (ImGui::Selectable(std::to_string(e->getID()).c_str(), selected,
                                              ImGuiSelectableFlags_SpanAllColumns))
                            selectedEntityId = e->getID();

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", e->getName().data());

                        ImGui::TableSetColumnIndex(2);
                        auto p = e->getPosition();
                        ImGui::Text("(%.1f, %.1f, %.1f)", p.x, p.y, p.z);

                        ImGui::TableSetColumnIndex(3);
                        auto r = e->getRotation();
                        ImGui::Text("(%.1f, %.1f, %.1f)", r.x, r.y, r.z);

                        ImGui::TableSetColumnIndex(4);
                        auto s = e->getScale();
                        ImGui::Text("(%.1f, %.1f, %.1f)", s.x, s.y, s.z);

                        if (auto rb = e->getRigidBody()) {
                            ImVec2 velCol = ImGui::GetCursorPos();
                            ImGui::TableSetColumnIndex(5);
                            float speed = glm::length(rb->getVelocity());
                            ImGui::Text("%.2f", speed);

                            ImGui::TableSetColumnIndex(6);
                            ImGui::Text("%.2f", rb->getMass());
                        }

                        ImGui::TableSetColumnIndex(7);
                        ImGui::Text("%s", e->getMeshId().data());

                        ImGui::TableSetColumnIndex(8);
                        ImGui::Text("%s", e->getMaterialId().data());

                        ImGui::TableSetColumnIndex(9);
                        ImGui::Text("%s", e->getShaderId().data());
                    }

                    ImGui::EndTable();
                }

                // Background color of the world
                ImGui::SeparatorText("World Visuals");
                static ImVec4 clear = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
                if (ImGui::ColorEdit4("Background Colour", (float *)(&clear))) {
                    // Update the renderer's clear color
                }

            } else {
                ImGui::TextUnformatted("No world data available");
            }
            ImGui::End();
        }

        std::string_view WorldProperties::GetName() const { return name; }

    } // namespace GUI
} // namespace Arche
