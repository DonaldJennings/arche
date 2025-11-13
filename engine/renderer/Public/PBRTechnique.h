#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "IrenderTechnique.h"

namespace Arche {
    namespace Render {

        class PBRTechnique : public IRenderTechnique {
            std::shared_ptr<Shader> shader;

          public:
            PBRTechnique(std::shared_ptr<Shader> s) : shader(std::move(s)) {}

            std::shared_ptr<Shader> getShader() const override { return shader; }

            void applyGlobals(IRenderBackend &backend, const Camera &camera) override {
                backend.setUniformMat4("uView", camera.GetViewMatrix());
                backend.setUniformMat4("uProj", camera.GetProjectionMatrix());

                backend.setUniformVec3("uCameraPos", camera.GetPosition());
                backend.setUniformVec3("uLightPos", glm::vec3(4, 8, 4));
                backend.setUniformVec3("uLightColor", glm::vec3(300, 300, 300));
            }

            void applyMaterial(IRenderBackend &backend, const Material &mat) override {
                backend.setUniformVec3("uAlbedo", mat.getVec3s().at("uAlbedo"));
                backend.setUniform1f("uMetallic", mat.getFloats().at("uMetallic"));
                backend.setUniform1f("uRoughness", mat.getFloats().at("uRoughness"));
                backend.setUniform1f("uAO", mat.getFloats().at("uAO"));
            }
        };

    } // namespace Render
} // namespace Arche