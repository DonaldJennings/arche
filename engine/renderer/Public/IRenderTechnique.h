#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "Shader.h"
#include "IRenderBackend.h"

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

    } // namespace Render
} // namespace Arche