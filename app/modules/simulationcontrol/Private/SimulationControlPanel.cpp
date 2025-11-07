#include "SimulationControlPanel.h"

#include <imgui.h>
#include <UIContext.h>
#include <World.h>

namespace Arche {
    namespace GUI {

        SimulationControlPanel::SimulationControlPanel(std::shared_ptr<UIContext> contextIn)
            : name{"SimulationControl"}, context(std::move(contextIn)) {}

        void SimulationControlPanel::Draw() {
            ImGui::Begin(name.c_str());

            ImGui::BeginGroup();

            ImVec2 btnSize(140.0f, 42.0f);

            if (ImGui::Button("Play", btnSize)) {
                if (context) context->runSimulation();
            }
            ImGui::SameLine();
            if (ImGui::Button("Pause", btnSize)) {
                if (context) context->pauseSimulation();
            }
            ImGui::SameLine();
            if (ImGui::Button("Step", btnSize)) {
                if (context && context->worldSystem() && context->worldSystem()->getWorld()) {
                    auto w = context->worldSystem()->getWorld();
                    float dt = w->stepDuration();
                    w->step(dt);
                    if (context && context->logger()) {
                        ARCHE_LOG_INFO(context->logger(), "World stepped by " + std::to_string(dt) + " seconds");
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset", btnSize)) {
                if (context && context->worldSystem()) {
                    Arche::Core::WorldConfig config;
                    config.gravity = Arche::Math::Vector3D(0.0f, 98.1f, 0.0f);
                    config.stepDuration = 1.0f / 60.0f;
                    config.maxSubSteps = 5;
                    config.deterministic = true;
                    auto newWorld = Arche::Scene::World::Create(config);
                    context->worldSystem()->setWorld(newWorld);
                    if (context && context->logger()) {
                        ARCHE_LOG_INFO(context->logger(), "World has been reset to default configuration");
                    }
                }
            }

            ImGui::EndGroup();
            ImGui::End();
        }

        std::string_view SimulationControlPanel::GetName() const { return name; }

    } // namespace GUI
} // namespace Arche
