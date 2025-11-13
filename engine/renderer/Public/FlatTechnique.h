#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "IRenderTechnique.h"
#include "Camera.h"
#include "Material.h"

namespace Arche {
    namespace Render {

        class FlatTechnique : public IRenderTechnique {
          public:
            explicit FlatTechnique(std::shared_ptr<Shader> shader) : m_shader(std::move(shader)) {}

            std::shared_ptr<Shader> getShader() const override { return m_shader; }

            // Upload view/projection (convert double matrices to float if Camera stores double)
            void applyGlobals(IRenderBackend &backend, const Render::Camera &camera) override {
                glm::mat4 view  = glm::mat4(camera.GetViewMatrix());
                glm::mat4 proj  = glm::mat4(camera.GetProjectionMatrix());
                backend.setUniformMat4("uView", view);
                backend.setUniformMat4("uProj", proj);
            }

            // Safely upload material parameters
            void applyMaterial(IRenderBackend &backend, const Material &mat) override {
                // Try explicit vec4 parameter first, fallback to baseColor
                const auto &vec4s = mat.getVec4s();
                auto vcIt = vec4s.find("uBaseColor");
                glm::vec4 color = (vcIt != vec4s.end()) ? vcIt->second : mat.getBaseColor();
                backend.setUniformVec4("uBaseColor", color);

                // Try explicit float parameter first, fallback to material point size
                const auto &floats = mat.getFloats();
                auto psIt = floats.find("uPointSize");
                float pointSize = (psIt != floats.end()) ? psIt->second : mat.getPointSize();
                backend.setUniform1f("uPointSize", pointSize);
            }

          private:
            std::shared_ptr<Shader> m_shader;
        };

    } // namespace Render
} // namespace Arche