#pragma once
#include <glm/glm.hpp>
#include <memory>

#include "Camera.h"
#include "GlobalSettings.h"
#include "IRenderBackend.h"
#include "IRenderPass.h"
#include "Material.h"
#include "Mesh.h"
#include "ResourceRegistry.h"
#include "Shader.h"

namespace Arche::Render {

    class ShadowPass : public IRenderPass {
      public:
        ShadowPass(int resolution = 2048) : m_resolution(resolution) {}

        void initialise(IRenderBackend &backend) override;

        void render(const RenderView &view, IRenderBackend &backend, const Camera &camera, ResourceRegistry &resources,
                    RenderPassSettings &settings) override;

        void shutdown(IRenderBackend &backend) override {}

      private:
        int m_resolution;
        glm::mat4 m_lightSpaceMatrix{1.0f};
        unsigned int m_shadowMapID{0};
        bool m_initialised{false};
    };

} // namespace Arche::Render
