#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "RenderScene.h"
#include "Shader.h"
#include "IRenderBackend.h"
#include "ResourceRegistry.h"
#include "GlobalSettings.h"

namespace Arche {
    namespace Render {

        /**
         * @brief Shadow mapping configuration and state.
         */
        struct ShadowSettings {
            unsigned int shadowMapID{0};
            glm::mat4    lightSpaceMatrix{1.0f};
            int          resolution{2048};
            bool         enabled{false};
        };

        /**
         * @brief Global settings passed to render passes.
         */
        class RenderPassSettings {
          public:
            RenderPassSettings() = default;
            ShadowSettings       shadowSettings;
            Core::RenderSettings globalSettings;
        };

        /**
         * @brief Per-frame view data passed to each render pass.
         *
         * opaqueObjects is now a list of RenderObjects (mesh ID, material ID,
         * transform) rather than raw IEntity pointers, so render passes have
         * no dependency on the scene layer.
         */
        struct RenderView {
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            glm::vec3 cameraPosition;

            std::vector<RenderObject> opaqueObjects;      ///< Solid renderables
            std::vector<RenderObject> transparentObjects; ///< Transparent renderables
        };

        /**
         * @brief Abstract interface for rendering passes.
         */
        class IRenderPass {
          public:
            virtual ~IRenderPass() = default;

            virtual void initialise(IRenderBackend &backend) = 0;

            virtual void render(const RenderView &view, IRenderBackend &backend,
                                const Camera &camera, ResourceRegistry &resources,
                                RenderPassSettings &settings) = 0;

            virtual void shutdown(IRenderBackend &backend) = 0;
        };

    } // namespace Render
} // namespace Arche
