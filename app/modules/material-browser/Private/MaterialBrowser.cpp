
#include "MaterialBrowser.h"
#include <ResourceRegistry.h>
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

        void MaterialBrowser::Draw() {
            ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_None);

            auto &materialRegistry = context->renderer()->resources();

            if (materialRegistry.getAllMaterials().empty()) {
                ImGui::Text("No materials loaded.");
                return;
            }

            for (const auto &[name, material] : materialRegistry.getAllMaterials()) {
                if (ImGui::TreeNode(name.c_str())) {

                    std::string shaderName{material->getShaderName()};
                    if (!shaderName.empty()) {
                        ImGui::Text("Shader: %s", shaderName.c_str());
                    } else {
                        ImGui::Text("Shader: None");
                    }

                    ImGui::Separator();

                    // Float properties
                    const std::unordered_map<std::string, float> &floats = material->getFloats();
                    if (!floats.empty()) {
                        if (ImGui::TreeNode("Float Properties")) {
                            for (auto &[n, v] : floats) {
                                ImGui::BulletText("%s = %.3f", n.c_str(), v);
                            }
                            ImGui::TreePop();
                        }
                    }

                    // Vec3 properties
                    const std::unordered_map<std::string, glm::vec3> &vec3s = material->getVec3s();
                    if (!vec3s.empty()) {
                        if (ImGui::TreeNode("Vec3 Properties")) {
                            for (auto &[n, v] : vec3s) {
                                ImGui::BulletText("%s = (%.3f, %.3f, %.3f)", n.c_str(), v.x, v.y, v.z);
                            }
                            ImGui::TreePop();
                        }
                    }

                    // Vec4 properties
                    const std::unordered_map<std::string, glm::vec4> &vec4s = material->getVec4s();
                    if (!vec4s.empty()) {
                        if (ImGui::TreeNode("Vec4 Properties")) {
                            for (auto &[n, v] : vec4s) {
                                ImGui::BulletText("%s = (%.3f, %.3f, %.3f, %.3f)", n.c_str(), v.x, v.y, v.z, v.w);
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
            }

            ImGui::End();
        }

        std::string_view MaterialBrowser::GetName() const { return name; }

    } // namespace GUI
} // namespace Arche
