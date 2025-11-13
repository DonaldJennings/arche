#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "IRenderTechnique.h"
#include "Camera.h"
#include "Material.h"

namespace Arche {
    namespace Render {
        class BlinnPhongTechnique : public IRenderTechnique {
            std::shared_ptr<Shader> shader;

          public:
            explicit BlinnPhongTechnique(std::shared_ptr<Shader> s) : shader(std::move(s)) {}

            std::shared_ptr<Shader> getShader() const override { return shader; }

            void applyGlobals(IRenderBackend &backend, const Camera &camera) override {
                // Convert double -> float matrices and vectors
                backend.setUniformMat4("uView", glm::mat4(camera.GetViewMatrix()));
                backend.setUniformMat4("uProj", glm::mat4(camera.GetProjectionMatrix()));
                backend.setUniformVec3("uCameraPos", glm::vec3(camera.GetPosition()));

                backend.setUniformVec3("uLightPos",   glm::vec3(5.0f, 5.0f, 5.0f));
                backend.setUniformVec3("uLightColor", glm::vec3(1.0f, 1.0f, 1.0f));
            }

            void applyMaterial(IRenderBackend &backend, const Material &mat) override {
                // Base color: use explicit param if present, else material's baseColor
                glm::vec4 base = mat.getBaseColor();
                if (const auto &v4 = mat.getVec4s(); !v4.empty()) {
                    auto it = v4.find("uBaseColor");
                    if (it != v4.end()) base = it->second;
                }
                backend.setUniformVec4("uBaseColor", base);

                // Shininess: safe lookup with defaults and multiple key aliases
                float shininess = 32.0f;
                if (const auto &f = mat.getFloats(); !f.empty()) {
                    if (auto it = f.find("uShininess"); it != f.end()) shininess = it->second;
                    else if (auto it2 = f.find("shininess"); it2 != f.end()) shininess = it2->second;
                }
                backend.setUniform1f("uShininess", shininess);
            }
        };
    } // namespace Render
} // namespace Arche