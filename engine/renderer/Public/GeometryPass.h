#pragma once
#include "IRenderPass.h"
#include "ResourceRegistry.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Arche {
    namespace Render {

        class GeometryPass : public IRenderPass {
          public:
            void initialise(IRenderBackend &backend) override {
                // Initialization code for the geometry pass
            }

            void render(const RenderView &view, IRenderBackend &backend, const Camera &camera,
                        ResourceRegistry &resources, RenderPassSettings &settings) override {

                // Apply camera matrices globally
                backend.setViewProjection(view.viewMatrix, view.projectionMatrix);

                // Opaque geometry
                for (const auto &e : view.opaqueObjects) {
                    drawEntity(e, resources, backend, settings, view.cameraPosition);
                }
            }

            void shutdown(IRenderBackend &backend) override {
                // Cleanup code for the geometry pass
            }

          private:
            void uploadMaterialProperties(IRenderBackend &backend, const Material &material) {
                // Floats
                for (const auto &pair : material.getFloats()) {
                    backend.setUniform1f(pair.first.c_str(), pair.second);
                }
                // Vec3
                for (const auto &pair : material.getVec3s()) {
                    backend.setUniformVec3(pair.first.c_str(), pair.second);
                }
                // Vec4
                for (const auto &pair : material.getVec4s()) {
                    backend.setUniformVec4(pair.first.c_str(), pair.second);
                }
                // If you support textures in Material, bind them here (albedo/roughness/metallic/normal, etc.)
            }

            void drawEntity(std::shared_ptr<Scene::IEntity> entity, const ResourceRegistry &resources,
                            IRenderBackend &backend, RenderPassSettings &settings, glm::vec3 cameraPos) {
                if (!entity) return;

                // Fetch mesh + material
                auto mesh = resources.getMesh(entity->getMeshId());
                auto material = resources.getMaterial(entity->getMaterialId());
                if (!mesh || !material) return;

                // Shader from material
                backend.setShader(resources.getShader(material->getShaderName()));

                
                // Camera
                backend.setUniformVec3("uCameraPos", cameraPos);

                // Light color
                backend.setUniformVec3("uLightColor", glm::vec3(1.0f)); // unit intensity

                // Shadows
                if (settings.shadowSettings.enabled) {
                    backend.setUniformMat4("uLightSpaceMatrix", settings.shadowSettings.lightSpaceMatrix);
                    // Bind depth texture to a stable unit; the backend should set uShadowMap to this unit internally.
                    backend.bindTexture("uShadowMap", settings.shadowSettings.shadowMapID, 5);
                }

                // Provide sane defaults for PBR (material upload will override if present)
                backend.setUniformVec3("uAlbedo", glm::vec3(0.8f));
                backend.setUniform1f("uMetallic", 0.0f);
                backend.setUniform1f("uRoughness", 0.5f);
                backend.setUniform1f("uAO", 1.0f);


                // Upload material uniforms (overrides defaults set earlier)
                uploadMaterialProperties(backend, *material);

                // Build model matrix (include rotation if your IEntity exposes it)
                glm::mat4 model = glm::translate(glm::mat4(1.0f), entity->getPosition());
                // If rotation exists: model *= glm::mat4_cast(entity->getRotation());
                model = glm::scale(model, entity->getScale());

                backend.drawMesh(*mesh, model);
            }
        };

    } // namespace Render
} // namespace Arche