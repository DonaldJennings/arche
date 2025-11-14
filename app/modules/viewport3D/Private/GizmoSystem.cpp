#include "GizmoSystem.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <glm/glm.hpp>

namespace Arche {
    namespace GUI {

        namespace {
            struct Ray {
                glm::dvec3 origin;
                glm::dvec3 direction;
            };
        } // namespace

        GizmoSystem::GizmoSystem()
            : m_mode(GizmoMode::Translate), m_hoveredAxis(GizmoAxis::None), m_activeAxis(GizmoAxis::None),
              m_isDragging(false) {}

        GizmoResult GizmoSystem::update(std::shared_ptr<Render::Camera> camera, const ImVec2 &viewportPos,
                                                     const ImVec2 &viewportSize, const glm::vec3 &objectPosition, const glm::vec3& objectScale) {
            ImGuiIO &io = ImGui::GetIO();
            glm::dvec3 axes[] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
            glm::dvec3 objectPosD = objectPosition;
            GizmoResult result;

            bool isShiftDown = io.KeyShift;

            // --- Start Dragging ---
            if ((m_hoveredAxis != GizmoAxis::None || isShiftDown) && io.MouseClicked[0]) {
                m_isDragging = true;
                m_activeAxis = isShiftDown ? GizmoAxis::All : m_hoveredAxis;
                m_dragStartPosition = objectPosition;
                m_dragStartScale = objectScale;

                if (m_activeAxis == GizmoAxis::All) { // Uniform Scale
                    m_dragStartOffset = io.MousePos.x;
                } else { // Translate
                    Ray mouseRay;
                    mouseRay.origin = camera->GetPosition();
                    glm::vec2 mousePos = {(io.MousePos.x - viewportPos.x) / viewportSize.x * 2.0f - 1.0f,
                                          1.0f - (io.MousePos.y - viewportPos.y) / viewportSize.y * 2.0f};
                    glm::dmat4 invVP = glm::inverse(camera->GetProjectionMatrix() * camera->GetViewMatrix());
                    glm::dvec4 near = invVP * glm::dvec4(mousePos.x, mousePos.y, -1.0, 1.0);
                    mouseRay.direction = glm::normalize(glm::dvec3(near) / near.w - mouseRay.origin);

                    Ray axisRay = {objectPosD, axes[static_cast<int>(m_activeAxis) - 1]};

                    glm::dvec3 c1 = mouseRay.origin - axisRay.origin;
                    double a = glm::dot(mouseRay.direction, mouseRay.direction);
                    double b = glm::dot(mouseRay.direction, axisRay.direction);
                    double c = glm::dot(axisRay.direction, axisRay.direction);
                    double d = glm::dot(mouseRay.direction, c1);
                    double e = glm::dot(axisRay.direction, c1);
                    double t = (b * e - c * d) / (a * c - b * b);

                    m_dragStartOffset = glm::dot(mouseRay.origin + mouseRay.direction * t - objectPosD, axisRay.direction);
                }
            }

            // --- Handle Dragging ---
            if (m_isDragging && io.MouseDown[0]) {
                if (m_activeAxis == GizmoAxis::All) { // Uniform Scale
                    float mouseDeltaX = io.MousePos.x - (float)m_dragStartOffset;
                    float scaleFactor = 1.0f + mouseDeltaX * 0.005f; // Sensitivity factor
                    result.newScale = m_dragStartScale * scaleFactor;
                } else { // Translate
                    Ray mouseRay;
                    mouseRay.origin = camera->GetPosition();
                    glm::vec2 mousePos = {(io.MousePos.x - viewportPos.x) / viewportSize.x * 2.0f - 1.0f,
                                          1.0f - (io.MousePos.y - viewportPos.y) / viewportSize.y * 2.0f};
                    glm::dmat4 invVP = glm::inverse(camera->GetProjectionMatrix() * camera->GetViewMatrix());
                    glm::dvec4 near = invVP * glm::dvec4(mousePos.x, mousePos.y, -1.0, 1.0);
                    mouseRay.direction = glm::normalize(glm::dvec3(near) / near.w - mouseRay.origin);

                    Ray axisRay = {m_dragStartPosition, axes[static_cast<int>(m_activeAxis) - 1]};

                    glm::dvec3 c1 = mouseRay.origin - axisRay.origin;
                    double a = glm::dot(mouseRay.direction, mouseRay.direction);
                    double b = glm::dot(mouseRay.direction, axisRay.direction);
                    double c = glm::dot(axisRay.direction, axisRay.direction);
                    double d = glm::dot(mouseRay.direction, c1);
                    double e = glm::dot(axisRay.direction, c1);
                    double t = (b * e - c * d) / (a * c - b * b);

                    double currentOffset = glm::dot(mouseRay.origin + mouseRay.direction * t - (glm::dvec3)m_dragStartPosition, axisRay.direction);
                    double delta = currentOffset - m_dragStartOffset;

                    result.newPosition = m_dragStartPosition + (glm::vec3)axisRay.direction * (float)delta;
                }
            }

            // --- Stop Dragging ---
            if (m_isDragging && io.MouseReleased[0]) {
                m_isDragging = false;
                m_activeAxis = GizmoAxis::None;
            }

            return result;
        }

        void GizmoSystem::draw(ImDrawList *drawList, std::shared_ptr<Render::Camera> camera, const ImVec2 &viewportPos,
                               const ImVec2 &viewportSize, const glm::vec3 &objectPosition) {
            const float gizmoSize = 0.15f * glm::length(objectPosition - glm::vec3(camera->GetPosition()));
            const float handleThickness = 4.0f;
            const float arrowheadSize = 10.0f;
            const float hitDistanceThreshold = 12.0f;

            glm::vec3 axes[] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
            ImU32 colors[] = {IM_COL32(255, 50, 50, 255), IM_COL32(50, 255, 50, 255), IM_COL32(50, 50, 255, 255)};
            ImU32 hoveredColors[] = {IM_COL32(255, 150, 150, 255), IM_COL32(150, 255, 150, 255),
                                     IM_COL32(150, 150, 255, 255)};
            ImU32 scaleColor = IM_COL32(255, 255, 100, 255);

            ImVec2 screenOrigin, screenEnd;
            m_hoveredAxis = GizmoAxis::None;

            // Draw center cube for uniform scaling
            ImVec2 center;
            if (worldToScreen(objectPosition, camera->GetViewMatrix(), camera->GetProjectionMatrix(), viewportPos, viewportSize, center)) {
                float cubeSize = 10.0f;
                ImVec2 min = { center.x - cubeSize / 2, center.y - cubeSize / 2 };
                ImVec2 max = { center.x + cubeSize / 2, center.y + cubeSize / 2 };
                bool isHoveringCenter = ImGui::IsMouseHoveringRect(min, max);
                if (isHoveringCenter && !m_isDragging) {
                    m_hoveredAxis = GizmoAxis::All;
                }
                drawList->AddRectFilled(min, max, (m_hoveredAxis == GizmoAxis::All || m_activeAxis == GizmoAxis::All) ? scaleColor : IM_COL32(200, 200, 200, 255));
            }

            for (int i = 0; i < 3; ++i) {
                GizmoAxis currentAxis = static_cast<GizmoAxis>(i + 1);
                if (worldToScreen(objectPosition, camera->GetViewMatrix(), camera->GetProjectionMatrix(), viewportPos,
                                  viewportSize, screenOrigin) &&
                    worldToScreen(objectPosition + axes[i] * gizmoSize, camera->GetViewMatrix(),
                                  camera->GetProjectionMatrix(), viewportPos, viewportSize, screenEnd)) {

                    float dist = getDistanceToLine(ImGui::GetMousePos(), screenOrigin, screenEnd);
                    if (dist < hitDistanceThreshold && !m_isDragging) {
                        m_hoveredAxis = currentAxis;
                    }

                    ImU32 color =
                        (m_hoveredAxis == currentAxis || m_activeAxis == currentAxis) ? hoveredColors[i] : colors[i];
                    drawList->AddLine(screenOrigin, screenEnd, color, handleThickness);

                    ImVec2 dir = {screenEnd.x - screenOrigin.x, screenEnd.y - screenOrigin.y};
                    float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
                    if (length > 0) {
                        dir.x /= length;
                        dir.y /= length;
                        ImVec2 p1 = {screenEnd.x - dir.x * arrowheadSize, screenEnd.y - dir.y * arrowheadSize};
                        ImVec2 p2 = {p1.x - dir.y * arrowheadSize * 0.4f, p1.y + dir.x * arrowheadSize * 0.4f};
                        ImVec2 p3 = {p1.x + dir.y * arrowheadSize * 0.4f, p1.y - dir.x * arrowheadSize * 0.4f};
                        drawList->AddTriangleFilled(screenEnd, p2, p3, color);
                    }
                }
            }
        }

        bool GizmoSystem::worldToScreen(const glm::vec3 &worldPos, const glm::mat4 &view, const glm::mat4 &proj,
                                        const ImVec2 &canvasP0, const ImVec2 &canvasSize, ImVec2 &outScreenPos) {
            glm::dmat4 dView = view;
            glm::dmat4 dProj = proj;
            glm::dvec4 clipPos = dProj * dView * glm::dvec4(worldPos, 1.0);
            if (clipPos.w <= 0.0)
                return false;
            glm::dvec3 ndc = glm::dvec3(clipPos) / clipPos.w;
            outScreenPos.x = canvasP0.x + (float)((ndc.x + 1.0) * 0.5 * canvasSize.x);
            outScreenPos.y = canvasP0.y + (float)((1.0 - (ndc.y + 1.0) * 0.5) * canvasSize.y);
            return (ndc.x >= -1.0 && ndc.x <= 1.0 && ndc.y >= -1.0 && ndc.y <= 1.0);
        }

        float GizmoSystem::getDistanceToLine(const ImVec2 &p, const ImVec2 &a, const ImVec2 &b) {
            ImVec2 ap = {p.x - a.x, p.y - a.y};
            ImVec2 ab = {b.x - a.x, b.y - a.y};
            float ab2 = ab.x * ab.x + ab.y * ab.y;
            if (ab2 == 0.0f) {
                float dx = p.x - a.x;
                float dy = p.y - a.y;
                return sqrtf(dx * dx + dy * dy);
            }
            float ap_dot_ab = ap.x * ab.x + ap.y * ab.y;
            float t = ap_dot_ab / ab2;
            t = std::max(0.0f, std::min(1.0f, t));
            ImVec2 closest = {a.x + ab.x * t, a.y + ab.y * t};
            float dx = p.x - closest.x;
            float dy = p.y - closest.y;
            return sqrtf(dx * dx + dy * dy);
        }
    } // namespace GUI
} // namespace Arche