#pragma once

#include <IPanel.h>
#include <SpatialTransform.h>
#include <UIContext.h>
#include <Vector3D.h>
#include <World.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <optional>

namespace {
    struct EditState {
        double posX = 0, posY = 0, mass = 1, scale=1;
        std::uint64_t editingId = 0;
    };
    std::optional<std::uint64_t> selectedBodyId;
    std::optional<EditState> editState;
    ImVec2 cameraOffset{0.0f, 0.0f}; // <--- Add this line
    ImVec2 ImVec2Subtract(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x - b.x, a.y - b.y); }
} // namespace

namespace Arche {
    namespace GUI {

        class Viewport2DPanel : public IPanel {
          public:
            Viewport2DPanel(std::shared_ptr<Arche::GUI::UIContext> panelContext) : name{"2DViewport"}, context(panelContext) {}

            void Draw() override {
                ImGui::Begin(name.c_str());

                // Access the world through the WorldSystem in the UIContext
                auto worldSystem = context->worldSystem;
                if (!worldSystem) {
                    ImGui::Text("World system not available.");
                    ImGui::End();
                    return;
                }
                auto world = worldSystem->getWorld();
                if (!world) {
                    ImGui::Text("World not available.");
                    ImGui::End();
                    return;
                }

                ImVec2 canvasP0 = ImGui::GetCursorScreenPos();      // Top-left
                ImVec2 canvasSize = ImGui::GetContentRegionAvail(); // Size of the canvas
                if (canvasSize.x < 50.0f)
                    canvasSize.x = 50.0f;
                if (canvasSize.y < 50.0f)
                    canvasP0.y = 50.0f;
                ImVec2 canvasP1 = ImVec2(canvasP0.x + canvasSize.x, canvasP0.y + canvasSize.y); // Bottom-right

                // Draw border and background color
                ImDrawList *drawList = ImGui::GetWindowDrawList();
                ImU32 topColor = IM_COL32(10, 30, 80, 255);      // Dark blue (top)
                ImU32 bottomColor = IM_COL32(135, 206, 250, 255); // Light blue (bottom)
                drawList->AddRectFilledMultiColor(
                    canvasP0, canvasP1,
                    topColor, topColor, // Top-left, Top-right
                    bottomColor, bottomColor // Bottom-left, Bottom-right
                );

                if (!context->simulationIsPaused()) {
                    world->step(world->stepDuration());
                }
                auto view = world->view();

                ImVec2 mousePos{ImGui::GetIO().MousePos};
                bool mouseClicked{ImGui::IsMouseClicked(0)};

                drawList->PushClipRect(canvasP0, canvasP1, true);

                // Draw particles and handle selection by ID
                for (const auto &body : view.bodies) {
                    ImVec2 pos = ImVec2(
                        canvasP0.x + body.transform.getPosition().x() - cameraOffset.x,
                        canvasP0.y + body.transform.getPosition().y() - cameraOffset.y
                    );
                    float baseRadius = 5.0f;
                    float scale = static_cast<float>(body.transform.getScale().x());
                    float radius = baseRadius * scale;

                    drawList->AddCircleFilled(pos, radius, IM_COL32(200, 100, 100, 255));

                    if (mouseClicked && ImGui::IsWindowHovered() && ImLengthSqr(ImVec2Subtract(mousePos, pos)) < radius * radius) {
                        std::ostringstream oss;
                        oss << "Clicked on particle id " << body.id << " at position (" << body.transform.getPosition().x() << ", " << body.transform.getPosition().y() << ")";
                        ARCHE_LOG_INFO(context->logger, oss.str());
                        selectedBodyId = body.id;
                        // Initialize edit state for this body
                        auto p = body.transform.getPosition();
                        editState = EditState{p.x(), p.y(), body.mass, body.transform.getScale().x(), body.id};
                    }
                }

                drawList->PopClipRect();

                if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup("ViewportContextMenu");
                }

                if (ImGui::BeginPopup("ViewportContextMenu")) {
                    if (ImGui::MenuItem("Add Particle Here")) {
                        // Calculate position relative to canvas
                        ImVec2 relPos = ImVec2(mousePos.x - canvasP0.x + cameraOffset.x, mousePos.y - canvasP0.y + cameraOffset.y);
                        Arche::Math::SpatialTransform transform;
                        transform.setPosition(Arche::Math::Vector3D(relPos.x, relPos.y, 0.0f));
                        world->createParticle(transform, 1.0f); // Default mass, or prompt for mass if desired
                    }
                    ImGui::EndPopup();
                }

                auto selectedIt = selectedBodyId ? std::find_if(view.bodies.begin(), view.bodies.end(), [&](const auto &b) { return b.id == *selectedBodyId; }) : view.bodies.end();

                if (selectedBodyId && selectedIt != view.bodies.end()) {
                    ImGui::OpenPopup("EditObjectPopup");
                }

                if (ImGui::BeginPopupModal("EditObjectPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    if (selectedIt != view.bodies.end() && editState && editState->editingId == selectedIt->id) {
                        ImGui::InputDouble("Position X: ", &editState->posX);
                        ImGui::InputDouble("Position Y: ", &editState->posY);
                        ImGui::InputDouble("Mass: ", &editState->mass);
                        ImGui::InputDouble("Scale: ", &editState->scale);

                        if (ImGui::Button("Apply")) {
                            world->setObjectPosition(selectedIt->id, Arche::Math::Vector3D(editState->posX, editState->posY, 0.0f));
                            world->setObjectMass(selectedIt->id, editState->mass);
                            world->setObjectScale(selectedIt->id, editState->scale);
                            ImGui::CloseCurrentPopup();
                            selectedBodyId.reset();
                            editState.reset();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel")) {
                            ImGui::CloseCurrentPopup();
                            selectedBodyId.reset();
                            editState.reset();
                        }
                    } else {
                        ImGui::CloseCurrentPopup();
                        selectedBodyId.reset();
                        editState.reset();
                    }
                    ImGui::EndPopup();
                }

                ImGui::Dummy(ImVec2(0, 8));
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 4));

                // Row of buttons at the bottom of the panel
                if (ImGui::Button("Reset", ImVec2(120, 0))) {
                    Reset();
                }
                ImGui::SameLine();
                if (ImGui::Button(context->simulationIsPaused() ? "Resume" : "Pause", ImVec2(120, 0))) {
                    context->toggleSimulationState();
                }
                ImGui::SameLine();
                if (ImGui::Button("Step", ImVec2(120, 0))) {
                    if (context->simulationIsPaused()) world->step(world->stepDuration());
                }

                ImGuiIO& io = ImGui::GetIO();
                bool viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

                // Only pan if the viewport is focused and not editing a text field
                if (viewportFocused && !ImGui::IsAnyItemActive()) {
                    ImGuiConfigFlags oldFlags = io.ConfigFlags;
                    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard; // Temporarily disable keyboard navigation

                    float panSpeed = 10.0f;
                    ImGuiKeyData* leftKey   = ImGui::GetKeyData(ImGuiKey_LeftArrow);
                    ImGuiKeyData* rightKey  = ImGui::GetKeyData(ImGuiKey_RightArrow);
                    ImGuiKeyData* upKey     = ImGui::GetKeyData(ImGuiKey_UpArrow);
                    ImGuiKeyData* downKey   = ImGui::GetKeyData(ImGuiKey_DownArrow);

                    if (leftKey && leftKey->Down)   cameraOffset.x -= panSpeed;
                    if (rightKey && rightKey->Down) cameraOffset.x += panSpeed;
                    if (upKey && upKey->Down)       cameraOffset.y -= panSpeed;
                    if (downKey && downKey->Down)   cameraOffset.y += panSpeed;

                    io.ConfigFlags = oldFlags; // Restore config flags
                }

                ImGui::End();
            }

            std::string_view GetName() const override { return name; }

          private:
            std::string name;
            std::shared_ptr<Arche::GUI::UIContext> context;

            void Reset() {
                ARCHE_LOG_WARNING(context->logger, "Resetting the world");
                Arche::Core::WorldConfig config;
                config.gravity = Arche::Math::Vector3D(0.0f, 98.1f, 0.0f); // Gravity pointing downwards
                config.stepDuration = 1.0f / 60.0f;                        // 60 Hz
                config.maxSubSteps = 5;
                config.deterministic = true;

                auto newWorld = Arche::Scene::World::Create(config);
                context->worldSystem->setWorld(newWorld);
            }
        };

    } // namespace GUI
} // namespace Arche