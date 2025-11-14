#pragma once
#include "Viewport3D.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <Camera.h>
#include <CameraController.h>
#include <UIContext.h>
#include <WorldSystem.h>

#include <algorithm>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "SphereEntity.h"

#include "CubeEntity.h"

#include "PlaneEntity.h"

#include <glm/glm.hpp>
#include <limits>

namespace {
    struct EditState {
        double posX = 0, posY = 0, posZ=0, mass = 1, scale = 1;
        bool isStatic = false, useGravity = true;
        std::uint64_t editingId = 0;
    };

    std::optional<std::uint64_t> selectedBodyId;
    std::optional<EditState> editState;
    ImVec2 cameraOffset{0.0f, 0.0f};

    inline ImVec2 ImVec2Subtract(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x - b.x, a.y - b.y); }
} // namespace

namespace Arche {
    namespace GUI {

        Viewport3DPanel::Viewport3DPanel(std::shared_ptr<Arche::GUI::UIContext> panelContext)
            : name{"3DViewport"}, context(std::move(panelContext)),
              cameraController{std::make_shared<CameraController>()} {
            // nothing else for now
        }

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
                unsigned int tex = context->renderer()->getRenderTextureID();
                // resize handling (recreate FBO if canvas size changed)
                int rw = static_cast<int>(canvasSize.x);
                int rh = static_cast<int>(canvasSize.y);

                glm::ivec2 canvasSizeVec{rw, rh};
                if (rw > 0 && rh > 0 && (canvasSizeVec != context->renderer()->getBackBufferSize())) {
                    context->renderer()->onResize(canvasSizeVec);

                    // Immediately render current world into new framebuffer so UI samples valid pixels
                    auto worldSystem = context->worldSystem();
                    if (worldSystem) {
                        context->renderer()->render(*worldSystem, context->globalSettings().getRenderSettings());
                    }
                }

                if (tex != 0) {
                    drawList->PushClipRect(canvasP0, canvasP1, true);
                    ImVec2 uv0(0.0f, 1.0f);
                    ImVec2 uv1(1.0f, 0.0f);
                    drawList->AddImage((void *)(intptr_t)tex, canvasP0, canvasP1, uv0, uv1);
                    drawList->PopClipRect();
                }
            }
        }

        // Helper: find the nearest body under mouse and prepare edit state if clicked
        void Viewport3DPanel::HandleSelectionAndMarkers(const ImVec2 &canvasP0, const ImVec2 &canvasP1,
                                                const ImVec2 &canvasSize, const ImVec2 &mousePos,
                                                bool mouseClicked, const glm::dmat4 &viewMatrix,
                                                const glm::dmat4 &projectionMatrix, ImDrawList *drawList) {
            auto worldSystem = context->worldSystem();
            if (!worldSystem) return;

            // Only allow selecting inside the viewport
            const bool hoveringCanvas = ImGui::IsMouseHoveringRect(canvasP0, canvasP1, true);

            auto view = worldSystem->view();
            drawList->PushClipRect(canvasP0, canvasP1, true);

            double bestDepth = std::numeric_limits<double>::infinity();
            std::optional<std::uint64_t> bestBodyId;

            for (const auto &body : view.bodies) {
                if (!body) continue;

                const glm::dvec3 posW = body->getPosition();
                glm::dvec4 worldPos{posW.x, posW.y, posW.z, 1.0};

                // View space for depth
                glm::dvec4 viewPosition4{viewMatrix * worldPos};
                double viewZ{viewPosition4.z};
                if (!(viewZ < 0.0)) continue;

                // Clip -> NDC center
                glm::dvec4 clipC{projectionMatrix * viewPosition4};
                if (clipC.w == 0.0) continue;
                glm::dvec3 ndcC{glm::dvec3(clipC) / clipC.w};
                if (ndcC.x < -1.0 || ndcC.x > 1.0 || ndcC.y < -1.0 || ndcC.y > 1.0 || ndcC.z < -1.0 || ndcC.z > 1.0) continue;

                // Project a point offset by scale in world X to estimate screen radius
                glm::dvec3 sxW = posW + glm::dvec3(body->getScale().x, 0.0, 0.0);
                glm::dvec4 clipE = projectionMatrix * (viewMatrix * glm::dvec4(sxW, 1.0));
                glm::dvec2 screenC{
                    canvasP0.x + static_cast<float>(((ndcC.x + 1.0) * 0.5) * canvasSize.x),
                    canvasP0.y + static_cast<float>(((1.0 - ((ndcC.y + 1.0) * 0.5)) * canvasSize.y))
                };
                glm::dvec2 screenE = screenC; // default
                if (clipE.w != 0.0) {
                    glm::dvec3 ndcE = glm::dvec3(clipE) / clipE.w;
                    screenE = {
                        canvasP0.x + static_cast<float>(((ndcE.x + 1.0) * 0.5) * canvasSize.x),
                        canvasP0.y + static_cast<float>(((1.0 - ((ndcE.y + 1.0) * 0.5)) * canvasSize.y))
                    };
                }

                // Screen-space radius from projected offset (fallback to 8px minimum)
                float radius = std::max(8.0f, static_cast<float>(glm::length(screenE - screenC)));
                float maxAllowed = std::min(canvasSize.x, canvasSize.y) * 0.5f;
                radius = std::clamp(radius, 4.0f, maxAllowed);

                // Hit test
                float distSqr = ImLengthSqr(ImVec2Subtract(mousePos, ImVec2(screenC.x, screenC.y)));
                if (distSqr <= radius * radius) {
                    double depthKey = -viewZ;
                    if (depthKey < bestDepth) {
                        bestDepth = depthKey;
                        bestBodyId = body->getID();
                    }
                }
            }

            if (hoveringCanvas && mouseClicked && bestBodyId) {
                auto localSelectedBodyId = bestBodyId.value();
                auto selectedIt = std::find_if(view.bodies.begin(), view.bodies.end(),
                    [&](const auto &b) { return b && b->getID() == localSelectedBodyId; });

                if (selectedIt != view.bodies.end() && (*selectedIt)) {
                    auto &e = *selectedIt;
                    auto p = e->getPosition();
                    auto rb = e->getRigidBody();

                    // Safe defaults if there is no rigid body (e.g., planes)
                    double mass    = rb ? static_cast<double>(rb->getMass()) : 1.0;
                    bool   isStat  = rb ? rb->isStatic() : false;
                    bool   useGrav = rb ? rb->isUsingGravity() : false;

                    editState = EditState{p.x, p.y, p.z, mass, e->getScale().x, isStat, useGrav, e->getID()};
                    selectedBodyId = localSelectedBodyId;
                    ARCHE_LOG_INFO(context->logger(), "Selected entity id " + std::to_string(e->getID()));
                }
            }

            drawList->PopClipRect();
        }

        // Helper: camera input and movement handling (pan/rotate/zoom/keyboard)
        void Viewport3DPanel::HandleCameraInput(const ImVec2 &canvasP0, const ImVec2 &canvasP1,
                                                const ImVec2 &canvasSize,
                                                std::shared_ptr<Arche::Render::Camera> camera) {
            if (!cameraController)
                return;

            if (camera != attachedCamera) {
                cameraController->detachCamera();
                cameraController->attachCamera(camera);
                attachedCamera = camera;
            }

            bool hoveringCanvas = ImGui::IsMouseHoveringRect(canvasP0, canvasP1, true);
            bool allowControl = !ImGui::IsAnyItemActive();

            auto worldSystem = context->worldSystem();

            if (ImGui::IsKeyPressed(ImGuiKey_Insert)) {
                ARCHE_LOG_INFO(context->logger(), "Resetting camera to default position.");
                cameraController->reset();
                cameraController->updateCamera();
            }

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

                    if (context && context->renderer() && worldSystem) {
                        context->renderer()->render(*worldSystem, context->globalSettings().getRenderSettings());
                    }
                }
            }
        }

        // Helper: context menu (Add Particle Here)
        void Viewport3DPanel::HandleContextMenu(const ImVec2 &canvasP0, const ImVec2 &canvasSize,
                                                const ImVec2 &mousePos, std::shared_ptr<Arche::Render::Camera> camera) {
            auto worldSystem = context->worldSystem();
            if (!worldSystem)
                return;

            if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                ImGui::OpenPopup("ViewportContextMenu");
            }

            if (ImGui::BeginPopup("ViewportContextMenu")) {
                if (ImGui::BeginMenu("Add...")) {

                    // Build world ray from cursor (canvas-relative)
                    glm::dmat4 viewM = camera->GetViewMatrix();
                    glm::dmat4 projM = camera->GetProjectionMatrix();
                    glm::dmat4 invVP = glm::inverse(projM * viewM);

                    // Normalized device coordinates
                    double ndcX = ((mousePos.x - canvasP0.x) / canvasSize.x) * 2.0 - 1.0;
                    double ndcY = 1.0 - ((mousePos.y - canvasP0.y) / canvasSize.y) * 2.0;

                    glm::dvec4 nearP = invVP * glm::dvec4(ndcX, ndcY, -1.0, 1.0);
                    glm::dvec4 farP = invVP * glm::dvec4(ndcX, ndcY, 1.0, 1.0);
                    nearP /= nearP.w;
                    farP /= farP.w;

                    glm::dvec3 rayOrigin = camera->GetPosition();
                    glm::dvec3 rayDir = glm::normalize(glm::dvec3(farP - nearP));

                    // Intersect with ground plane Y = 0 (fallback to fixed distance if behind/parallel)
                    glm::dvec3 spawnWorldPos;
                    {
                        double planeY = 0.0;
                        double denom = rayDir.y;
                        if (std::abs(denom) > 1e-6) {
                            double t = (planeY - rayOrigin.y) / denom;
                            if (t > 0.0) {
                                spawnWorldPos = rayOrigin + rayDir * t;
                            } else {
                                spawnWorldPos = rayOrigin + rayDir * 20.0;
                            }
                        } else {
                            spawnWorldPos = rayOrigin + rayDir * 20.0;
                        }
                    }

                    if (ImGui::MenuItem("Sphere")) {
                        auto sphere = std::make_shared<Scene::SphereEntity>(10.0f, spawnWorldPos);
                        worldSystem->addEntity(sphere);
                    }
                    if (ImGui::MenuItem("Cube")) {
                        auto cube = std::make_shared<Scene::CubeEntity>(glm::vec3(10.f), spawnWorldPos);
                        worldSystem->addEntity(cube);
                    }
                    if (ImGui::MenuItem("Plane")) {
                        // Horizontal plane at clicked point
                        auto plane = std::make_shared<Scene::PlaneEntity>(glm::vec3(0.0f, 1.0f, 0.0f),
                                                                          static_cast<float>(spawnWorldPos.y));
                        plane->setPosition(glm::vec3(spawnWorldPos));       // keep X/Z from click
                        plane->setScale(glm::vec3(10.0f));                  // size in your mesh units
                        worldSystem->addEntity(plane);
                    }
                    if (ImGui::MenuItem("Light")) {
                        ARCHE_LOG_WARNING(context->logger(), "Lights are not yet implemented.");
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }
        }

        // Helper: edit object modal popup
        void Viewport3DPanel::HandleEditPopup(const ImVec2 &canvasP0, const ImVec2 &canvasSize) {
            auto worldSystem = context->worldSystem();
            if (!worldSystem)
                return;

            auto view = worldSystem->view();

            // Use cbegin/cend and compare against cend(). Avoid calling getID() on shared_ptr directly.
            auto selectedIt = selectedBodyId ? std::find_if(view.bodies.cbegin(), view.bodies.cend(),
                                                            [&](const std::shared_ptr<Arche::Scene::IEntity> &e) {
                                                                return e && e->getID() == *selectedBodyId;
                                                            })
                                             : view.bodies.cend();

            if (selectedBodyId && selectedIt != view.bodies.cend()) {
                ImGui::OpenPopup("EditObjectPopup");
            }

            if (ImGui::BeginPopupModal("EditObjectPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                if (selectedIt != view.bodies.cend() && editState && editState->editingId == (*selectedIt)->getID()) {

                    ImGui::InputDouble("Position X: ", &editState->posX);
                    ImGui::InputDouble("Position Y: ", &editState->posY);
                    ImGui::InputDouble("Position Z: ", &editState->posZ); // Z is not editable here
                    ImGui::InputDouble("Mass: ", &editState->mass);
                    ImGui::InputDouble("Scale: ", &editState->scale);
                    ImGui::Checkbox("Static body", &editState->isStatic);
                    ImGui::Checkbox("Gravity enabled", &editState->useGravity);



                    if (ImGui::Button("Save")) {
                        const auto id = (*selectedIt)->getID();
                        const float currentZ = (*selectedIt)->getPosition().z;

                        worldSystem->updateEntityPosition(id, glm::vec3(editState->posX, editState->posY, editState->posZ));
                        worldSystem->updateEntityScale(id, glm::vec3(static_cast<float>(editState->scale)));

                        if (auto rb = (*selectedIt)->getRigidBody()) {
                            worldSystem->updateEntityMass(id, static_cast<float>(editState->mass));
                            worldSystem->updateEntityStaticState(id, editState->isStatic);
                            worldSystem->updateEntityImpactedByGravity(id, editState->useGravity);
                        }

                        ImGui::CloseCurrentPopup();
                        editState.reset();
                        selectedBodyId.reset();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        ImGui::CloseCurrentPopup();
                        editState.reset();
                        selectedBodyId.reset();
                    }
                } else {
                    ImGui::CloseCurrentPopup();
                    editState.reset();
                }
                ImGui::EndPopup();
            }
        }

        // Helper: draw camera overlay in top-left of viewport
        void Viewport3DPanel::DrawCameraOverlay(const ImVec2 &canvasP0, const ImVec2 &canvasSize,
                                                std::shared_ptr<Arche::Render::Camera> camera) {
            // Prepare position text
            glm::dvec3 camPos = camera->GetPosition();
            double pitch = camera->GetPitch();
            double yaw = camera->GetYaw();
            double distance = 0.0;
            if (cameraController)
                distance = cameraController->getDistance();

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2);

            std::string lineTitle = "Camera";
            oss.str("");
            oss.clear();
            oss << "Pos: (" << std::fixed << std::setprecision(2) << camPos.x << ", " << camPos.y << ", " << camPos.z
                << ")";

            std::string linePos = oss.str();
            oss.str("");
            oss.clear();
            oss << "Pitch: " << std::fixed << std::setprecision(1) << pitch << " deg   Yaw: " << yaw << " deg";

            std::string lineAng = oss.str();
            oss.str("");
            oss.clear();
            std::string lineDist = oss.str();

            std::vector<std::string> lines{lineTitle, linePos, lineAng, lineDist};

            // Layout + style
            ImGuiIO &io3 = ImGui::GetIO();
            float dpiScale3 = io3.DisplayFramebufferScale.x;
            if (!(dpiScale3 > 0.0f))
                dpiScale3 = 1.0f;
            float uiScale3 = dpiScale3;

            // Padding and metrics
            ImVec2 padding{8.0f * uiScale3, 6.0f * uiScale3};

            float baseFontSize = ImGui::GetFontSize();
            float lineHeight = ImGui::GetTextLineHeight() * 0.95f;

            float widest = 0.0f;
            for (const auto &l : lines) {
                ImVec2 ts = ImGui::CalcTextSize(l.c_str());
                if (ts.x > widest)
                    widest = ts.x;
            }
            float boxW = widest + padding.x * 2.0f;
            float boxH = static_cast<float>(lines.size()) * lineHeight + padding.y * 2.0f;

            // Position the overlay inside the viewport (top-left with small margin)
            ImVec2 boxMin = ImVec2(canvasP0.x + 8.0f * uiScale3, canvasP0.y + 8.0f * uiScale3);
            ImVec2 boxMax = ImVec2(boxMin.x + boxW, boxMin.y + boxH);

            // Draw clipped to viewport
            ImDrawList *dl = ImGui::GetWindowDrawList();
            dl->PushClipRect(canvasP0, ImVec2(canvasP0.x + canvasSize.x, canvasP0.y + canvasSize.y), true);

            // Background and border
            ImU32 bgCol = IM_COL32(20, 24, 28, 220);
            ImU32 borderCol = IM_COL32(110, 120, 130, 200);
            ImU32 titleCol = IM_COL32(220, 220, 220, 230);
            ImU32 textCol = IM_COL32(200, 200, 200, 220);

            float rounding = 6.0f * uiScale3;
            dl->AddRectFilled(boxMin, boxMax, bgCol, rounding);
            dl->AddRect(boxMin, boxMax, borderCol, rounding, 0, 1.0f * uiScale3);

            // Draw title with slightly bolder color and a subtle separator
            ImVec2 textPos = ImVec2(boxMin.x + padding.x, boxMin.y + padding.y);
            dl->AddText(ImGui::GetFont(), baseFontSize * 1.0f, textPos, titleCol, lines[0].c_str());
            // Separator line
            float sepY = textPos.y + lineHeight;
            dl->AddLine(ImVec2(boxMin.x + padding.x * 0.5f, sepY + 4.0f * uiScale3),
                        ImVec2(boxMax.x - padding.x * 0.5f, sepY + 4.0f * uiScale3), IM_COL32(120, 120, 130, 80),
                        1.0f * uiScale3);

            // Remaining lines
            float y = sepY + 8.0f * uiScale3;
            for (size_t i = 1; i < lines.size(); ++i) {
                ImVec2 tp = ImVec2(boxMin.x + padding.x, y);
                dl->AddText(ImGui::GetFont(), baseFontSize * 0.9f, tp, textCol, lines[i].c_str());
                y += lineHeight;
            }

            dl->PopClipRect();
        }

        void Viewport3DPanel::Draw() {
            ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_None);

            auto camera = context->renderer()->getMainCamera();

            // Determine UI scale for high-DPI displays (use framebuffer scale)
            ImGuiIO &io = ImGui::GetIO();
            float dpiScale = io.DisplayFramebufferScale.x;
            if (!(dpiScale > 0.0f))
                dpiScale = 1.0f;
            float uiScale = dpiScale;

            // Access world system
            auto worldSystem = context->worldSystem();
            if (!worldSystem) {
                ImGui::Text("World system not available.");
                ImGui::End();
                return;
            }

            // Reserve canvas
            ImVec2 canvasP0 = ImGui::GetCursorScreenPos();
            ImVec2 canvasSize = ImGui::GetContentRegionAvail();
            if (canvasSize.x < 50.0f)
                canvasSize.x = 50.0f;
            if (canvasSize.y < 50.0f)
                canvasSize.y = 50.0f;
            ImVec2 canvasP1 = ImVec2(canvasP0.x + canvasSize.x, canvasP0.y + canvasSize.y);

            ImDrawList *drawList = ImGui::GetWindowDrawList();

            // Background + renderer texture
            DrawBackgroundAndRenderer(canvasP0, canvasP1, canvasSize, drawList, camera);

            // --- Interaction: selection, add, edit, draw markers ---
            ImVec2 mousePos{ImGui::GetIO().MousePos};
            bool mouseClicked{ImGui::IsMouseClicked(0)};

            glm::dmat4 viewMatrix{camera->GetViewMatrix()};
            glm::dmat4 projectionMatrix{camera->GetProjectionMatrix()};

            // Selection & overlays
            HandleSelectionAndMarkers(canvasP0, canvasP1, canvasSize, mousePos, mouseClicked, viewMatrix,
                                      projectionMatrix, drawList);

            // Camera input handling (pan/rotate/zoom/move)
            HandleCameraInput(canvasP0, canvasP1, canvasSize, camera);

            // Context menu (Add Particle)
            HandleContextMenu(canvasP0, canvasSize, mousePos, camera);

            // Edit popup modal
            HandleEditPopup(canvasP0, canvasSize);

            // --- Camera overlay drawn last so it appears on top of renderer image ---
            DrawCameraOverlay(canvasP0, canvasP1, camera);

            ImGuiIO &io2 = ImGui::GetIO();
            bool viewportFocused = ImGui::IsWindowFocused();

            ImGui::End();
        }

        std::string_view Viewport3DPanel::GetName() const { return name; }

        void Viewport3DPanel::Reset() { ARCHE_LOG_WARNING(context->logger(), "Resetting 3D viewport (no-op)"); }

    } // namespace GUI
} // namespace Arche