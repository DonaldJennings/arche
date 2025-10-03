#pragma once

#include <IPanel.h>
#include <SpatialTransform.h>
#include <Vector3D.h>
#include <World.h>
#include <imgui.h>
#include <iostream>

namespace Arche {
    namespace GUI {

        class Viewport2DPanel : public IPanel {
          public:
            Viewport2DPanel(std::unique_ptr<Arche::Scene::World> worldIn) : name{"2DViewport"}, world{std::move(worldIn)} {}

            void Draw() override {
                ImGui::Begin(name.c_str());
                ImVec2 canvasP0 = ImGui::GetCursorScreenPos();      // Top-left
                ImVec2 canvasSize = ImGui::GetContentRegionAvail(); // Size of the canvas
                if (canvasSize.x < 50.0f)
                    canvasSize.x = 50.0f;

                if (canvasSize.y < 50.0f)
                    canvasP0.y = 50.0f;

                ImVec2 canvasP1 = ImVec2(canvasP0.x + canvasSize.x, canvasP0.y + canvasSize.y); // Bottom-right

                // Draw border and background color
                ImDrawList *drawList = ImGui::GetWindowDrawList();
                drawList->AddRectFilled(canvasP0, canvasP1, IM_COL32(50, 50, 50, 255));
                drawList->AddRect(canvasP0, canvasP1, IM_COL32(255, 255, 255, 255));

                world->step();

                auto view = world->view();

                // Draw particles
                for (const auto &body : view.bodies) {
                    ImVec2 pos = ImVec2(canvasP0.x + body.transform.getPosition().x(), canvasP0.y + body.transform.getPosition().y());
                    float radius = 5.0f; // Fixed radius for simplicity
                    drawList->AddCircleFilled(pos, radius, IM_COL32(200, 100, 100, 255));
                }

                if (ImGui::Button("Reset")) {
                    Reset();
                }

                if (ImGui::Button("Add Particle")) {
                    Arche::Math::SpatialTransform transform;
                    transform.setPosition(Arche::Math::Vector3D(canvasSize.x / 2, canvasSize.y / 2, 0.0f));
                    size_t numberOfParticlesBefore = world->view().bodies.size();
                    world->createParticle(transform, 1.0f);
                    assert(world->view().bodies.size() == numberOfParticlesBefore + 1);
                }

                ImGui::End();
            }

            std::string_view GetName() const override { return name; }

          private:
            std::string name;
            std::unique_ptr<Arche::Scene::World> world;

            void Reset() {
                Arche::Core::WorldConfig config;
                config.gravity = Arche::Math::Vector3D(0.0f, 98.1f, 0.0f); // Gravity pointing downwards
                config.stepDuration = 1.0f / 60.0f; // 60 Hz
                config.maxSubSteps = 5;
                config.deterministic = true;
                world = Arche::Scene::World::Create(config);
            }

        };

    } // namespace GUI
} // namespace Arche