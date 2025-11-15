#include "ShaderBrowser.h"
#include <ResourceRegistry.h>
#include <WorldSystem.h>
#include <imgui.h>

#include <algorithm> // for std::max, std::min
#include <array>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

static std::unique_ptr<Arche::GUI::ShaderSourcePopup> vertexPopup = nullptr;
static std::unique_ptr<Arche::GUI::ShaderSourcePopup> fragmentPopup = nullptr;
static bool drawVertexPopup = false;
static bool drawFragmentPopup = false;

namespace Arche {
    namespace GUI {

        void ShaderSourcePopup::Open() { ImGui::OpenPopup(id.c_str()); }

        void ShaderSourcePopup::Draw() {
            if (ImGui::BeginPopupModal(id.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

                ImGui::TextUnformatted(id.c_str());
                ImGui::Separator();

                ImGui::BeginChild("Scroll", ImVec2(800, 600), true);
                ImGui::TextUnformatted(text.c_str());
                ImGui::EndChild();

                if (ImGui::Button("Close")) {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }

        void ShaderBrowser::Draw() {
            ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_None);

            auto &shaderRegistry = context->renderer()->resources();

            if (shaderRegistry.getAllShaders().empty()) {
                ImGui::Text("No shaders loaded.");
                return;
            }

            for (const auto &[shaderName, shader] : shaderRegistry.getAllShaders()) {

                if (ImGui::TreeNode(shaderName.c_str())) {

                    if (ImGui::Button("View Vertex Source")) {
                        vertexPopup =
                            std::make_unique<ShaderSourcePopup>(shaderName + ".vert", shader->getSources().vertexGLSL);
                        vertexPopup->Open();
                    }

                    if (ImGui::Button("View Fragment Source")) {
                        fragmentPopup = std::make_unique<ShaderSourcePopup>(shaderName + ".frag",
                                                                            shader->getSources().fragmentGLSL);
                        fragmentPopup->Open();
                    }

                    // Draw and manage popups after the main window
                    if (vertexPopup) {
                        vertexPopup->Draw();
                    }

                    if (fragmentPopup) {
                        fragmentPopup->Draw();
                    }

                    ImGui::TreePop();
                }
            }

            ImGui::End();
        }

        std::string_view ShaderBrowser::GetName() const { return name; }

    } // namespace GUI
} // namespace Arche
