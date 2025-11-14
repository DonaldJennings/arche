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

        class IRenderTechnique
        {
          public:
            virtual ~IRenderTechnique() = default;

            virtual std::shared_ptr<Shader> getShader() const = 0;
            virtual void applyGlobals(IRenderBackend &backed, const Camera &camera) = 0;
            virtual void applyMaterial(IRenderBackend &backend, const Material &material) = 0;
        };

        class RenderPassSettings
        {
          public:
            RenderPassSettings() = default;
            bool isEnabled{true};
            glm::vec3 clearColor{0.1f, 0.1f, 0.1f};
            glm::vec3 ambientLight{0.2f, 0.2f, 0.2f};
            glm::vec3 sunColor{1.0f, 1.0f, 0.9f};
            glm::vec3 sunPosition{10.0f, 10.0f, 0.0f};

            bool isDebugMode{true};
            bool wireframe{false};
            bool showGrid{true};
            bool showAxes{true};

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
                                ResourceRegistry &resources, const Core::RenderSettings &settings) = 0;
            virtual void shutdown(IRenderBackend &backend) = 0;
        };

    } // namespace Render
} // namespace Arche