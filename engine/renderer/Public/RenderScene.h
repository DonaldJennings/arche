#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Arche {
    namespace Render {

        /**
         * @brief Data for a single renderable object.
         *
         * A lightweight, renderer-facing description of one visible object.
         * Contains only the data the renderer needs; no scene or physics state.
         */
        struct RenderObject {
            std::string meshId;       ///< Mesh resource name
            std::string materialId;   ///< Material resource name
            glm::mat4   transform;    ///< Precomputed model matrix (TRS)
        };

        /**
         * @brief Snapshot of the scene prepared for rendering.
         *
         * Built by WorldSystem each frame. Passed to RenderingSystem::render()
         * so the renderer has no direct dependency on WorldSystem.
         */
        struct RenderScene {
            std::vector<RenderObject> opaqueObjects; ///< All opaque renderables this frame
        };

    } // namespace Render
} // namespace Arche
