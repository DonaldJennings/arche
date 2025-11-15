#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <optional>

// Forward declarations
namespace Arche {
    namespace Render {
        class Camera;
    }
}
struct ImDrawList;
struct ImVec2;

namespace Arche {
    namespace GUI {

        enum class GizmoAxis { None, X, Y, Z, All }; // Add 'All' for uniform scaling
        enum class GizmoMode { Translate, Rotate, Scale };

        // Result struct to hold potential transform updates
        struct GizmoResult {
            std::optional<glm::vec3> newPosition;
            std::optional<glm::vec3> newScale;
        };

        class GizmoSystem {
        public:
            GizmoSystem();

            // Update function now takes position and scale, and returns a result struct
            GizmoResult update(
                std::shared_ptr<Render::Camera> camera,
                const ImVec2& viewportPos,
                const ImVec2& viewportSize,
                const glm::vec3& objectPosition,
                const glm::vec3& objectScale
            );

            void draw(ImDrawList* drawList, std::shared_ptr<Render::Camera> camera, const ImVec2& viewportPos, const ImVec2& viewportSize, const glm::vec3& objectPosition);

            bool isDragging() const { return m_isDragging; }
            bool isHoveringHandle() const { return m_hoveredAxis != GizmoAxis::None; }

            void GizmoSystem::resetHoverState() { m_hoveredAxis = GizmoAxis::None; }

            void setMode(GizmoMode mode) { m_mode = mode; }

        private:
            // State
            GizmoMode m_mode;
            GizmoAxis m_hoveredAxis;
            GizmoAxis m_activeAxis;
            bool m_isDragging;

            // Store initial state for drag operations
            glm::vec3 m_dragStartPosition;
            glm::vec3 m_dragStartScale;
            double m_dragStartOffset;

            // Math helpers
            static bool worldToScreen(const glm::vec3& worldPos, const glm::mat4& view, const glm::mat4& proj, const ImVec2& canvasP0, const ImVec2& canvasSize, ImVec2& outScreenPos);
            static float getDistanceToLine(const ImVec2& p, const ImVec2& a, const ImVec2& b);
        };

    } // namespace GUI
} // namespace Arche