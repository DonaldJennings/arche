#pragma once
#include "Viewport3D.h"
#include "GizmoSystem.h"

#include <cstdint>
#include <imgui.h>
#include <imgui_internal.h>

#include <Camera.h>
#include <CameraController.h>
#include <EditorSession.h>
#include <WorldSystem.h>

#include <algorithm>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "CubeEntity.h"
#include "PlaneEntity.h"
#include "SphereEntity.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <limits>

namespace {
    inline ImVec2 ImVec2Subtract(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x - b.x, a.y - b.y); }
    inline ImVec2 ImVec2Add(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x + b.x, a.y + b.y); }
} // namespace

namespace Arche {
    namespace GUI {

        Viewport3DPanel::Viewport3DPanel(std::shared_ptr<Arche::GUI::EditorSession> panelContext)
            : name{"3DViewport"}, context(std::move(panelContext)),
              cameraController{std::make_shared<CameraController>()}, m_gizmoSystem{std::make_unique<GizmoSystem>()} {}

        // Helper: draw background and renderer texture (handles resize & FBO recreate)
        void Viewport3DPanel::DrawBackgroundAndRenderer(const ImVec2 &canvasP0, const ImVec2 &canvasP1,
                                                        const ImVec2 &canvasSize, ImDrawList *drawList,
                                                        std::shared_ptr<Arche::Render::Camera> camera) {
            // Background (under renderer image)
            ImU32 bg = IM_COL32(30, 30, 40, 255);
            drawList->AddRectFilled(canvasP0, canvasP1, bg);
            drawList->AddRect(canvasP0, canvasP1, IM_COL32(80, 80, 90, 255), 0.0f, 0, 1.0f);

            // Draw renderer texture (if any) clipped to canvas
            if (context && context->renderer()) {
                uint64_t tex = context->renderer()->getRenderTextureID();
                // resize handling (recreate FBO if canvas size changed)
                int rw = static_cast<int>(canvasSize.x);
                int rh = static_cast<int>(canvasSize.y);

                glm::ivec2 canvasSizeVec{rw, rh};
                if (rw > 0 && rh > 0 && (canvasSizeVec != context->renderer()->getBackBufferSize())) {
                    context->renderer()->onResize(canvasSizeVec);

                    // Immediately render current world into new framebuffer so UI samples valid pixels
                    auto worldSystem = context->worldSystem();
                    if (worldSystem) {
                        context->renderer()->render(worldSystem->buildRenderScene(),
                                                    context->globalSettings().getRenderSettings());
                    }
                }

                if (tex != 0) {
                    drawList->PushClipRect(canvasP0, canvasP1, true);
                    bool flip = context->renderer()->needsRenderTextureYFlip();
                    drawList->AddImage((ImTextureID)tex, canvasP0, canvasP1,
                                       {0, flip ? 1.0f : 0.0f}, {1, flip ? 0.0f : 1.0f});
                    drawList->PopClipRect();
                }
            }
        }

        void Viewport3DPanel::HandleSelectionAndMarkers(const ImVec2 &canvasP0, const ImVec2 &canvasP1,
                                                        const ImVec2 &canvasSize, const ImVec2 &mousePos,
                                                        bool mouseClicked, const glm::dmat4 &viewMatrix,
                                                        const glm::dmat4 &projectionMatrix, ImDrawList *drawList) {
            auto worldSystem = context->worldSystem();
            if (!worldSystem)
                return;

            if (!mouseClicked)
                return;

            bool inside = ImRect(canvasP0, canvasP1).Contains(mousePos);
            if (!inside)
                return;

            auto view = worldSystem->view();

            double bestDepth = std::numeric_limits<double>::infinity();
            std::optional<uint64_t> bestHit = std::nullopt;

            for (auto &e : view.bodies) {
                if (!e)
                    continue;

                glm::dvec3 pos = e->getPosition();

                glm::dvec4 viewPos = viewMatrix * glm::dvec4(pos, 1.0);
                if (viewPos.z >= 0.0)
                    continue;

                glm::dvec4 clip = projectionMatrix * viewPos;
                if (clip.w == 0.0)
                    continue;

                glm::dvec3 ndc = glm::dvec3(clip) / clip.w;
                if (ndc.x < -1 || ndc.x > 1 || ndc.y < -1 || ndc.y > 1)
                    continue;

                // Screen center
                ImVec2 screenCenter{canvasP0.x + float((ndc.x + 1.0) * 0.5 * canvasSize.x),
                                    canvasP0.y + float((1.0 - (ndc.y + 1.0) * 0.5) * canvasSize.y)};

                // Project a point offset by radius
                float worldRadius = glm::length(e->getScale()) * 0.5f;
                glm::dvec3 edgeWorld = pos + glm::dvec3(worldRadius, 0, 0);

                glm::dvec4 edgeClip = projectionMatrix * (viewMatrix * glm::dvec4(edgeWorld, 1.0));
                ImVec2 screenEdge = screenCenter;
                if (edgeClip.w != 0.0) {
                    glm::dvec3 edgeNDC = glm::dvec3(edgeClip) / edgeClip.w;
                    screenEdge = {canvasP0.x + float((edgeNDC.x + 1.0) * 0.5 * canvasSize.x),
                                  canvasP0.y + float((1.0 - (edgeNDC.y + 1.0) * 0.5) * canvasSize.y)};
                }

                float radius_px = std::max(12.0f, ImGui::GetIO().DisplayFramebufferScale.x *
                                                      sqrtf(ImLengthSqr(ImVec2Subtract(screenEdge, screenCenter))));

                float dist = ImLengthSqr(ImVec2Subtract(mousePos, screenCenter));

                if (dist <= radius_px * radius_px) {
                    double depth = -viewPos.z;
                    if (depth < bestDepth) {
                        bestDepth = depth;
                        bestHit = e->getID();
                    }
                }
            }

            if (bestHit)
                context->setSelectedEntity(bestHit);
            else
                context->clearSelectedEntity();
        }

        // Helper: camera input and movement handling (pan/rotate/zoom/keyboard)
        void Viewport3DPanel::HandleCameraInput(const ImVec2 &canvasP0, const ImVec2 &canvasP1,
                                                const ImVec2 &canvasSize,
                                                std::shared_ptr<Arche::Render::Camera> camera) {
            if (!cameraController || m_gizmoSystem->isDragging())
                return;

            if (camera != attachedCamera) {
                cameraController->detachCamera();
                cameraController->attachCamera(camera);
                attachedCamera = camera;
            }

            bool mouseOverAnyWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

            if (mouseOverAnyWindow && !ImGui::IsWindowHovered()) {
                // Mouse is over *another* window (like color picker)
                return; // Do not process viewport input
            }

            bool hoveringCanvas = ImGui::IsMouseHoveringRect(canvasP0, canvasP1, true);
            bool allowControl = !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

            if (hoveringCanvas && allowControl) {
                // Pan / rotate with mouse
                ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;

                if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
                    cameraController->rotate(0.0f, 1.0f);
                    cameraController->updateOrientationOnly(); // only change direction
                }

                if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
                    cameraController->rotate(0.0f, -1.0f);
                    cameraController->updateOrientationOnly(); // only change direction
                }

                if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
                    cameraController->rotate(-1.0f, 0.0f);
                    cameraController->updateOrientationOnly(); // only change direction
                }

                if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
                    cameraController->rotate(1.0f, 0.0f);
                    cameraController->updateOrientationOnly(); // only change direction
                }

                float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f) {
                    cameraController->zoom(wheel);
                    cameraController->updateCamera();
                }

                ImGuiIO &cameraMovementIO{ImGui::GetIO()};

                float dt{cameraMovementIO.DeltaTime > 0.0f ? cameraMovementIO.DeltaTime : (1.0f / 60.0f)};

                float baseSpeed{cameraController->getDistance() > 0.0f ? cameraController->getDistance() * 0.5f : 1.0f};
                float speedMultiplier = ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 5.0f : 1.0f;
                float moveSpeed = baseSpeed * dt * speedMultiplier;

                float forward = 0.0f, right = 0.0f, up = 0.0f;
                if (ImGui::IsKeyDown(ImGuiKey_W))
                    forward += moveSpeed;
                if (ImGui::IsKeyDown(ImGuiKey_S))
                    forward -= moveSpeed;
                if (ImGui::IsKeyDown(ImGuiKey_D))
                    right += moveSpeed;
                if (ImGui::IsKeyDown(ImGuiKey_A))
                    right -= moveSpeed;
                if (ImGui::IsKeyDown(ImGuiKey_E))
                    up += moveSpeed;
                if (ImGui::IsKeyDown(ImGuiKey_Q))
                    up -= moveSpeed;

                if (forward != 0.0f || right != 0.0f || up != 0.0f) {
                    cameraController->moveLocal(forward, right, up);
                    cameraController->updateCamera();

                    if (context && context->renderer() && context->worldSystem()) {
                        context->renderer()->render(context->worldSystem()->buildRenderScene(),
                                                    context->globalSettings().getRenderSettings());
                    }
                }
            }
        }

        void Viewport3DPanel::DrawCameraViewGizmo(const ImVec2 &canvasP0, const ImVec2 &canvasSize,
                                                  std::shared_ptr<Arche::Render::Camera> camera, ImDrawList *drawList) {
            const float gizmoSize = 80.0f;
            const float margin = 20.0f;

            // Position in top-right corner
            ImVec2 center =
                ImVec2(canvasP0.x + canvasSize.x - gizmoSize / 2 - margin, canvasP0.y + gizmoSize / 2 + margin);

            // Get camera's rotation matrix
            glm::mat3 camRotation = glm::mat3_cast(camera->GetOrientation());

            struct Axis {
                glm::vec3 direction;
                const char *label;
                ImU32 color;
            };

            Axis axes[] = {{{1, 0, 0}, "X", IM_COL32(255, 0, 0, 255)},
                           {{0, 1, 0}, "Y", IM_COL32(0, 255, 0, 255)},
                           {{0, 0, 1}, "Z", IM_COL32(0, 0, 255, 255)}};

            // Draw axes lines and labels
            for (const auto &axis : axes) {
                // Transform axis direction by camera rotation
                glm::vec3 screenDir3 = camRotation * axis.direction;
                ImVec2 screenDir = ImVec2(screenDir3.x, -screenDir3.y); // Y is inverted in screen space

                // Draw line
                drawList->AddLine(
                    center,
                    ImVec2(center.x + screenDir.x * (gizmoSize / 2.5f), center.y + screenDir.y * (gizmoSize / 2.5f)),
                    axis.color, 2.0f);

                // Draw label
                ImVec2 textSize = ImGui::CalcTextSize(axis.label);
                ImVec2 textPos = ImVec2(center.x + screenDir.x * (gizmoSize / 2.0f) - textSize.x / 2,
                                        center.y + screenDir.y * (gizmoSize / 2.0f) - textSize.y / 2);
                drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), axis.label);
            }
        }

        // Helper: draw camera overlay in top-left of viewport
        void Viewport3DPanel::DrawSimulationControls(const ImVec2 &canvasP0, const ImVec2 &canvasSize) {
            if (!context) {
                return;
            }

            const float margin = 18.0f;
            ImVec2 overlayPos = ImVec2(canvasP0.x + canvasSize.x * 0.5f, canvasP0.y + canvasSize.y - margin);

            ImGui::SetNextWindowPos(overlayPos, ImGuiCond_Always, ImVec2(0.5f, 1.0f));
            ImGui::SetNextWindowBgAlpha(0.6f);
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                     ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 6.0f));
            if (ImGui::Begin("Simulation Controls##3DOverlay", nullptr, flags)) {
                bool paused = context->simulationIsPaused();
                const char *playPauseLabel = paused ? "Play" : "Pause";

                if (ImGui::Button(playPauseLabel, ImVec2(80.0f, 0.0f))) {
                    context->toggleSimulationState();
                }

                ImGui::SameLine();
                if (ImGui::Button("Reset", ImVec2(80.0f, 0.0f))) {
                    context->resetSimulationState();
                }

                ImGui::SameLine();
                ImGui::TextDisabled(paused ? "Paused" : "Running");
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
        }

        void Viewport3DPanel::DrawCameraOverlay(const ImVec2 &canvasP0, const ImVec2 &canvasSize,
                                                std::shared_ptr<Arche::Render::Camera> camera) {
            if (!camera) {
                return;
            }

            const float margin = 8.0f;
            ImVec2 overlayPos = ImVec2(canvasP0.x + margin, canvasP0.y + margin);

            ImGui::SetNextWindowPos(overlayPos, ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.55f);
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                     ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
            if (ImGui::Begin("Viewport Camera Info##3DOverlay", nullptr, flags)) {
                glm::dvec3 camPos = camera->GetPosition();
                ImGui::Text("Pos: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }

        void Viewport3DPanel::Draw() {
            ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_None);

            auto camera = context->renderer()->getMainCamera();
            if (!camera) {
                ImGui::Text("Main camera not set.");
                ImGui::End();
                return;
            }

            auto worldSystem = context->worldSystem();
            if (!worldSystem) {
                ImGui::Text("World system not available.");
                ImGui::End();
                return;
            }

            if (!context->getSelectedEntity()) {
                m_gizmoSystem->resetHoverState();
            }

            // Prepare canvas
            ImVec2 canvasP0 = ImGui::GetCursorScreenPos();
            ImVec2 canvasSize = ImGui::GetContentRegionAvail();
            canvasSize.x = std::max(canvasSize.x, 50.0f);
            canvasSize.y = std::max(canvasSize.y, 50.0f);
            ImVec2 canvasP1 = ImVec2Add(canvasP0, canvasSize);
            ImDrawList *drawList = ImGui::GetWindowDrawList();

            // Draw render output
            DrawBackgroundAndRenderer(canvasP0, canvasP1, canvasSize, drawList, camera);

            // Mouse/hover gating that respects other ImGui windows/popups (e.g., color picker)
            ImVec2 mousePos = ImGui::GetIO().MousePos;
            const bool viewportWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
            const bool mouseOverCanvas = ImGui::IsMouseHoveringRect(canvasP0, canvasP1, true);
            const bool uiBlocking = ImGui::IsAnyItemActive() || ImGui::IsAnyItemHovered();
            const bool viewportInteractable = viewportWindowHovered && mouseOverCanvas && !uiBlocking;

            bool mouseLeftClick = viewportInteractable && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

            glm::dmat4 viewMatrix = camera->GetViewMatrix();
            glm::dmat4 projectionMatrix = camera->GetProjectionMatrix();

            // ---------------------------------------------------------
            // 1. First draw gizmo (so we know what is hovered)
            // ---------------------------------------------------------
            auto selectedId = context->getSelectedEntity();
            if (selectedId) {
                auto view = worldSystem->view();
                auto it = std::find_if(view.bodies.begin(), view.bodies.end(),
                                       [&](const auto &e) { return e && e->getID() == *selectedId; });

                if (it != view.bodies.end()) {
                    m_gizmoSystem->draw(drawList, camera, canvasP0, canvasSize, (*it)->getPosition());
                }
            }

            // ---------------------------------------------------------
            // 2. Context menu
            // ---------------------------------------------------------
            HandleContextMenu(canvasP0, canvasSize, mousePos, camera);

            // ---------------------------------------------------------
            // 3. Camera input (block only while dragging gizmo)
            // ---------------------------------------------------------
            if (!m_gizmoSystem->isDragging()) {
                HandleCameraInput(canvasP0, canvasP1, canvasSize, camera);
            }

            // ---------------------------------------------------------
            // 4. Selection (only if not on gizmo + not dragging gizmo)
            // ---------------------------------------------------------
            bool allowSelection = !m_gizmoSystem->isHoveringHandle() && !m_gizmoSystem->isDragging();

            if (allowSelection) {
                HandleSelectionAndMarkers(canvasP0, canvasP1, canvasSize, mousePos, mouseLeftClick, viewMatrix,
                                          projectionMatrix, drawList);
            }

            // ---------------------------------------------------------
            // 5. Gizmo update (after selection, guaranteed entity exists)
            // ---------------------------------------------------------
            selectedId = context->getSelectedEntity();
            if (selectedId) {
                auto view = worldSystem->view();
                auto it = std::find_if(view.bodies.begin(), view.bodies.end(),
                                       [&](const auto &e) { return e && e->getID() == *selectedId; });

                if (it != view.bodies.end()) {
                    GizmoResult result =
                        m_gizmoSystem->update(camera, canvasP0, canvasSize, (*it)->getPosition(), (*it)->getScale());

                    if (result.newPosition)
                        worldSystem->updateEntityPosition((*it)->getID(), *result.newPosition);

                    if (result.newScale)
                        worldSystem->updateEntityScale((*it)->getID(), *result.newScale);
                }
            }

            // ---------------------------------------------------------
            // 6. Overlay last
            // ---------------------------------------------------------
            DrawSimulationControls(canvasP0, canvasSize);
            DrawCameraOverlay(canvasP0, canvasSize, camera);

            if (ImGui::IsKeyPressed(ImGuiKey_Insert)) {
                Reset();
            }

            ImGui::End();
        }

        std::string_view Viewport3DPanel::GetName() const { return name; }

        void Viewport3DPanel::HandleContextMenu(const ImVec2 &canvasP0, const ImVec2 &canvasSize,
                                                const ImVec2 &mousePos, std::shared_ptr<Arche::Render::Camera> camera) {
            auto worldSystem = context->worldSystem();
            if (!worldSystem)
                return;

            bool hoveringCanvas =
                ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) &&
                ImGui::IsMouseHoveringRect(canvasP0, ImVec2(canvasP0.x + canvasSize.x, canvasP0.y + canvasSize.y));

            // Open the context menu when right clicking inside the viewport
            if (hoveringCanvas && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
                ImGui::OpenPopup("ViewportContextMenu");

            if (ImGui::BeginPopup("ViewportContextMenu")) {
                // Build world ray from mouse coordinates
                glm::dmat4 viewM = camera->GetViewMatrix();
                glm::dmat4 projM = camera->GetProjectionMatrix();
                glm::dmat4 invVP = glm::inverse(projM * viewM);

                double ndcX = ((mousePos.x - canvasP0.x) / canvasSize.x) * 2.0 - 1.0;
                double ndcY = 1.0 - ((mousePos.y - canvasP0.y) / canvasSize.y) * 2.0;

                glm::dvec4 nearP = invVP * glm::dvec4(ndcX, ndcY, -1.0, 1.0);
                glm::dvec4 farP = invVP * glm::dvec4(ndcX, ndcY, 1.0, 1.0);
                nearP /= nearP.w;
                farP /= farP.w;

                glm::dvec3 rayOrigin = camera->GetPosition();
                glm::dvec3 rayDir = glm::normalize(glm::dvec3(farP - nearP));

                // Intersect ray with ground plane Y = 0
                glm::dvec3 spawnWorldPos;
                double denom = rayDir.y;
                if (std::abs(denom) > 1e-6) {
                    double t = (0.0 - rayOrigin.y) / denom;
                    if (t > 0.0)
                        spawnWorldPos = rayOrigin + rayDir * t;
                    else
                        spawnWorldPos = rayOrigin + rayDir * 20.0;
                } else {
                    spawnWorldPos = rayOrigin + rayDir * 20.0;
                }

                // --- Spawnable entities ---
                if (ImGui::MenuItem("Add Sphere")) {
                    auto sphere = std::make_shared<Scene::SphereEntity>(10.0f, spawnWorldPos);
                    worldSystem->addEntity(sphere);
                }

                if (ImGui::MenuItem("Add Cube")) {
                    auto cube = std::make_shared<Scene::CubeEntity>(glm::vec3(10.f), spawnWorldPos);
                    worldSystem->addEntity(cube);
                }

                if (ImGui::MenuItem("Add Plane")) {
                    auto plane = std::make_shared<Scene::PlaneEntity>(glm::vec3(0, 1, 0), spawnWorldPos.y);
                    worldSystem->addEntity(plane);
                }

                if (ImGui::MenuItem("Add Light")) {
                    ARCHE_LOG_WARNING(context->logger(), "Lights not yet implemented.");
                }

                ImGui::EndPopup();
            }
        }

        void Viewport3DPanel::Reset() {
            ARCHE_LOG_WARNING(context->logger(), "Resetting 3D viewport (no-op)");
            attachedCamera = context->renderer()->getMainCamera();
            cameraController->detachCamera();
            cameraController->attachCamera(attachedCamera);
            cameraController->reset();
        }

    } // namespace GUI
} // namespace Arche