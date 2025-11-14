#pragma once
#include <glm/glm.hpp>
#include <memory>

#include "Camera.h"
#include "IRenderBackend.h"
#include "IRenderPass.h"
#include "Material.h"
#include "Mesh.h"
#include "ResourceRegistry.h"
#include "Shader.h"

namespace Arche::Render {

    class DebugPass : public IRenderPass {
      public:
        DebugPass() = default;
        void initialise(IRenderBackend &backend) override {}; // Not implemented
        void render(const RenderView &view, IRenderBackend &backend, const Camera &camera, ResourceRegistry &resources,
                    const Core::RenderSettings &settings) override;
        void shutdown(IRenderBackend &backend) override {}; // Not implemented

      private:
        void drawGrid(IRenderBackend &backend, const glm::vec3 &cameraPosition, ResourceRegistry &resources);
        void drawAxes(IRenderBackend &backend, const glm::vec3 &cameraPosition, ResourceRegistry &resources);
    };

} // namespace Arche::Render
