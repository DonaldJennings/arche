#pragma once

#include <IPanel.h>
#include <memory>
#include <string>
#include <string_view>
#include <imgui.h>
#include <glm/glm.hpp>
#include "GizmoSystem.h"
#include "CameraController.h"
#include "UIContext.h"
#include "Camera.h"

namespace Arche {
    namespace GUI {
        class Viewport3DPanel : public IPanel {
          public:
            explicit Viewport3DPanel(std::shared_ptr<Arche::GUI::UIContext> panelContext);

            void Draw() override;
            std::string_view GetName() const override;

          private:
            std::string name;
            std::shared_ptr<Arche::GUI::UIContext> context;
            std::shared_ptr<CameraController> cameraController;
            std::shared_ptr<Arche::Render::Camera> attachedCamera;
            std::unique_ptr<Arche::GUI::GizmoSystem> m_gizmoSystem;

            void Reset();

            // Refactored helper methods (implementation in .cpp)
            void DrawBackgroundAndRenderer(const ImVec2 &canvasP0, const ImVec2 &canvasP1,
                                           const ImVec2 &canvasSize, ImDrawList *drawList,
                                           std::shared_ptr<Arche::Render::Camera> camera);

            void HandleSelectionAndMarkers(const ImVec2 &canvasP0, const ImVec2 &canvasP1,
                                           const ImVec2 &canvasSize, const ImVec2 &mousePos,
                                           bool mouseClicked, const glm::dmat4 &viewMatrix,
                                           const glm::dmat4 &projectionMatrix, ImDrawList *drawList);

            void HandleCameraInput(const ImVec2 &canvasP0, const ImVec2 &canvasP1, const ImVec2 &canvasSize,
                                   std::shared_ptr<Arche::Render::Camera> camera);

            void DrawSimulationControls(const ImVec2 &canvasP0, const ImVec2 &canvasSize);

            void DrawCameraOverlay(const ImVec2 &canvasP0, const ImVec2 &canvasSize,
                                   std::shared_ptr<Arche::Render::Camera> camera);

            void DrawCameraViewGizmo(const ImVec2 &canvasP0, const ImVec2 &canvasSize,
                                     std::shared_ptr<Arche::Render::Camera> camera, ImDrawList *drawList);  
        
            void HandleContextMenu(const ImVec2 &canvasP0, const ImVec2 &canvasSize, const ImVec2 &mousePos,
                                   std::shared_ptr<Arche::Render::Camera> camera);
        };
    } // namespace GUI
} // namespace Arche