#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "WorldProperties.h"

#include <imgui.h>
#include <WorldSystem.h>

#include <iostream>
#include <chrono>
#include <vector>
#include <array>
#include <numeric>
#include <algorithm> // for std::max, std::min


namespace Arche {
    namespace GUI {

        void WorldProperties::Draw() {
            ImGui::Begin(name.c_str());

            // Existing world info
            if (context && context->worldSystem()) {
                auto world = context->worldSystem();
                auto view{world->view()};

                ImGui::SeparatorText("Physics Properties");
                ImGui::Text("Number of Bodies: %zu", view.bodies.size());
                ImGui::Text("Simulation State: %s", context->simulationIsPaused() ? "Paused" : "Running");
                std::optional<glm::vec3> gravityOpt = world->getWorldGravity();

                if (gravityOpt.has_value()) {
                    const glm::vec3 &gravity = gravityOpt.value();
                    ImGui::Text("Gravity: (%.2f, %.2f, %.2f)", gravity.x, gravity.y, gravity.z);
                } else {
                    ImGui::Text("Gravity: N/A");
                }

                //Arche::Math::Vector3D gravity{world->gravity()};

                ImGui::SeparatorText("World Properties");
                //I//mGui::Text("Gravitational Force: (%.1f, %.1f, %.1f)", gravity.x(), gravity.y(), gravity.z());
                //ImGui::Text("Gravity Magnitude: %.2f", gravity.length());

                if (ImGui::BeginTable("Bodies", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Index");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Position");
                    ImGui::TableSetupColumn("Velocity");
                    ImGui::TableSetupColumn("Mass");
                    ImGui::TableHeadersRow();

                    for (std::shared_ptr<Arche::Scene::IEntity> entity : view.bodies) {
                        // You can access entity properties here if needed
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%zu", entity->getID());

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(entity->getName().data());

                        ImGui::TableSetColumnIndex(2);
                        const auto &pos = entity->getPosition();
                        ImGui::Text("(%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);

                        ImGui::TableSetColumnIndex(3);
                        auto rb = entity->getRigidBody();
                        if (rb) {
                            const glm::vec3 vel = rb->getVelocity();
                            ImGui::Text("(%.2f, %.2f, %.2f)", vel.x, vel.y, vel.z);
                        } else {
                            ImGui::Text("(N/A)");
                        }

                        ImGui::TableSetColumnIndex(4);
                        auto rb2 = entity->getRigidBody();
                        if (rb2) {
                            ImGui::Text("%.2f", rb2->getMass());
                        } else {
                            ImGui::Text("N/A");
                        }
                    }

                    ImGui::EndTable();
                }
                
                // Background color of the world
                ImGui::SeparatorText("World Visuals");
                static ImVec4 clear = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
                if (ImGui::ColorEdit4("Background Colour", (float*)(&clear))) {
                    // Update the renderer's clear color
                    context->renderer()->setClearColour(clear.x, clear.y, clear.z, clear.w);
                }

            } else {
                ImGui::TextUnformatted("No world data available");
            }
            ImGui::End();
        }

        std::string_view WorldProperties::GetName() const { return name; }

    } // namespace GUI
} // namespace Arche
