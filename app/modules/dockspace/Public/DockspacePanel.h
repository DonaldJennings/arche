#pragma once

#include "UIContext.h"
#include <IPanel.h>
#include <imgui.h>
#include <string_view>
#include <Theme.h>

namespace Arche {
    namespace GUI {
        class DockspacePanel : public IPanel {
          public:
            DockspacePanel(std::shared_ptr<UIContext> contextIn) : name{"Dockspace"}, context{std::move(contextIn)} {}

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

                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("Load Demo")) {
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
                    ImGui::EndMenuBar();
                }

                ImGui::PopStyleVar(2);

                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

                ImGui::End();
            }

            std::string_view GetName() const override { return name; }

          private:
            std::string name;
            std::shared_ptr<UIContext> context;

            void RunParticlesDemo(std::shared_ptr<Arche::Scene::World> world) {
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
        };
    } // namespace GUI
} // namespace Arche