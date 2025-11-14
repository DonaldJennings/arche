#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "Shader.h"
#include "IRenderBackend.h"
#include "ResourceRegistry.h"
#include "GlobalSettings.h"
#include <WorldSystem.h>

namespace Arche {
    namespace Render {

        struct ShadowSettings
        {
            unsigned int shadowMapID{0};
            glm::mat4 lightSpaceMatrix{1.0f};
            int resolution{2048};
            bool enabled{false};
        };
        class RenderPassSettings
        {
          public:
            RenderPassSettings() = default;
            ShadowSettings shadowSettings;
            Core::RenderSettings globalSettings;

          private:
        };

        struct RenderView {
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;

            glm::vec3 cameraPosition;

            std::vector<std::shared_ptr<Scene::IEntity>> opaqueObjects;
            std::vector<std::shared_ptr<Scene::IEntity>> transparentObjects;
        };


        class IRenderPass
        {
          public:
            virtual ~IRenderPass() = default;
            virtual void initialise(IRenderBackend &backend) = 0;
            virtual void render(const RenderView &view, IRenderBackend &backend, const Camera &camera,
                                ResourceRegistry &resources, RenderPassSettings &settings) = 0;
            virtual void shutdown(IRenderBackend &backend) = 0;
        };

    } // namespace Render
} // namespace Arche