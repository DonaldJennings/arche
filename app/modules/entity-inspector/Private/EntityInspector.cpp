#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "EntityInspector.h"

#include <WorldSystem.h>
#include <imgui.h>

#include <algorithm> // for std::max, std::min
#include <array>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <numeric>
#include <vector>

namespace Arche {
    namespace GUI {

        void EntityInspector::Draw() {
            ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_None);

            ImGui::Text("Entity Inspector Panel");
        
            if (context->getSelectedEntity())
            {
                ImGui::Text("Selected Entity ID: %llu", context->getSelectedEntity().value());

                // Find element in world system
                auto worldSystem = context->worldSystem();
                auto view = worldSystem->view();
                auto it = std::find_if(view.bodies.begin(), view.bodies.end(),
                                       [selectedID = context->getSelectedEntity().value()](const std::shared_ptr<Scene::IEntity>& entity) {
                                           return entity->getID() == selectedID;
                    });

                if (it != view.bodies.end())
                    {auto entity = *it;
                    glm::vec3 position = entity->getPosition();
                    glm::vec3 rotation = entity->getRotation();
                    glm::vec3 scale = entity->getScale();
                    ImGui::Text("Name: %s", std::string(entity->getName()).c_str());
                    ImGui::Text("Mesh ID: %s", std::string(entity->getMeshId()).c_str());
                    ImGui::Text("Material ID: %s", std::string(entity->getMaterialId()).c_str());
                    ImGui::Text("Shader ID: %s", std::string(entity->getShaderId()).c_str());
                    ImGui::Separator();
                    ImGui::Text("Transform:");
                    ImGui::InputFloat3("Position", glm::value_ptr(position));
                    ImGui::InputFloat3("Rotation", glm::value_ptr(rotation));
                    ImGui::InputFloat3("Scale", glm::value_ptr(scale));
                    // Update entity transform if changed
                    if (ImGui::Button("Apply Transform"))
                    {
                        entity->setPosition(position);
                        entity->setRotation(rotation);
                        entity->setScale(scale);
                    }
                }
                else
                {
                    ImGui::Text("Selected entity not found in world.");
                }
            }
            ImGui::End();
        }

        std::string_view EntityInspector::GetName() const { return name; }

    } // namespace GUI
} // namespace Arche
