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
#include <glm/gtc/type_ptr.hpp>

namespace Arche {
    namespace GUI {

        WorldProperties::WorldProperties(std::shared_ptr<UIContext> contextIn)
            : name{"World Properties"}, context{std::move(contextIn)} {}

        void WorldProperties::Draw() {
            ImGui::Begin(name.c_str());

            drawRendererConfiguration();
            drawEditorSettings();
            drawDirectionalLightSettings();
            drawWorldConfiguration();
            drawWorldInfo();

            ImGui::End();
        }

        std::string_view WorldProperties::GetName() const { return name; }

        void WorldProperties::drawRendererConfiguration() {
            auto &renderSettings = context->globalSettings().getRenderSettings();

            if (ImGui::CollapsingHeader("Renderer Configuration")) {
                ImGui::ColorEdit4("Clear Color", (float *)(&renderSettings.clearColor));
                ImGui::SliderFloat("Field of View", &renderSettings.fieldOfView, 30.0f, 120.0f);
                ImGui::Checkbox("VSync", &renderSettings.vsync);
            }
        }

        void WorldProperties::drawEditorSettings() {
            auto &renderSettings = context->globalSettings().getRenderSettings();

            if (ImGui::CollapsingHeader("Editor Settings")) {
                ImGui::Checkbox("Wireframe Mode", &renderSettings.wireframe);
                ImGui::Checkbox("Show Grid", &renderSettings.showGrid);
                ImGui::Checkbox("Show Axes", &renderSettings.showAxes);
            }
        }

        void WorldProperties::drawDirectionalLightSettings() {
            auto &settings{context->globalSettings().getRenderSettings()};
            auto &col = settings.directionalLightColor;
            auto &dir = settings.directionalLightDirection;
            auto &intensity = settings.directionalLightIntensity;
            auto &dist = settings.directionalLightDistance;

            if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::ColorEdit3("Color", glm::value_ptr(col));

                glm::vec3 d = dir;
                if (ImGui::DragFloat3("Direction", glm::value_ptr(d), 0.05f, -1.0f, 1.0f)) {
                    if (glm::dot(d, d) < 1e-4f) {
                        d = glm::vec3(0.0f, -1.0f, 0.0f);
                    }
                    dir = glm::normalize(d);
                }

                ImGui::DragFloat3("Intensity", glm::value_ptr(intensity), 0.1f, 0.0f, 100.0f);
                ImGui::DragFloat("Distance", &dist, 1.0f, 1.0f, 2000.0f);

                glm::vec3 lightPos = -dir * dist;
                ImGui::Text("Computed Light Pos: (%.2f, %.2f, %.2f)", lightPos.x, lightPos.y, lightPos.z);
            }
        }

        void WorldProperties::drawWorldConfiguration() {
            auto &worldSettings = context->globalSettings().getWorldSettings();

            if (ImGui::CollapsingHeader("World Configuration")) {
                ImGui::DragFloat3("Gravity", &worldSettings.gravity.x, 0.1f, -100.0f, 100.0f);
                ImGui::SliderFloat("Time Scale", &worldSettings.timeScale, 0.0f, 5.0f);
            }
        }

        void WorldProperties::drawWorldInfo() {
            if (!(context && context->worldSystem())) {
                ImGui::TextUnformatted("No world data available");
                return;
            }

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

            drawWorldVisuals();
        }

        void WorldProperties::drawWorldVisuals() {
            ImGui::SeparatorText("World Visuals");
            static ImVec4 clear = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
            if (ImGui::ColorEdit4("Background Colour", (float *)(&clear))) {
                // Update the renderer's clear color
            }
        }

    } // namespace GUI
} // namespace Arche
