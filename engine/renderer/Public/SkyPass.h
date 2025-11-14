#pragma once
#include <glm/glm.hpp>
#include <memory>

#include "Camera.h"
#include "IRenderBackend.h"
#include "IRenderPass.h"
#include "Material.h"
#include "Mesh.h"
#include "ResourceRegistry.h"
#include "GlobalSettings.h"
#include "Shader.h"

namespace Arche::Render {

    class SkyPass : public IRenderPass {
      public:
        SkyPass() = default;

        void initialise(IRenderBackend &backend) override;

        void render(const RenderView &view, IRenderBackend &backend, const Camera &camera, ResourceRegistry &resources,
                    RenderPassSettings &settings) override;

        void shutdown(IRenderBackend &backend) override;

      private:
        void uploaderUniformProperties(IRenderBackend &backend, glm::mat4 inverseViewProjection);

        glm::vec3 m_sunDirection{0.3f, 0.8f, 0.2f};
        glm::vec3 m_sunColor{5.0f, 5.0f, 4.8f};
    };

} // namespace Arche::Render
