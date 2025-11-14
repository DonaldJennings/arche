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
                        ResourceRegistry &resources, const Core::RenderSettings &settings) override {

                // Apply camera matrices globally
                backend.setViewProjection(view.viewMatrix, view.projectionMatrix);

                backend.setUniformVec3("uCameraPos", view.cameraPosition);
                backend.setUniformVec3("uLightColor", settings.ambientLight);
                backend.setUniformVec3("uLightPos", settings.ambientPosition);

                // Loop over all opaque entities (later you'll add transparent pass)
                for (const auto &e : view.opaqueObjects) {
                    drawEntity(e, resources, backend);
                }
            }

            void shutdown(IRenderBackend &backend) override {
                // Cleanup code for the geometry pass
            }

          private:
            void uploadMaterialProperties(IRenderBackend &backend, const Material &material) {
                // ------------------------------------------------------------
                // Upload float uniforms
                // ------------------------------------------------------------
                for (const auto &pair : material.getFloats()) {
                    const std::string &name = pair.first;
                    float value = pair.second;

                    backend.setUniform1f(name.c_str(), value);
                }

                // ------------------------------------------------------------
                // Upload vec3 uniforms
                // ------------------------------------------------------------
                for (const auto &pair : material.getVec3s()) {
                    const std::string &name = pair.first;
                    const glm::vec3 &value = pair.second;

                    backend.setUniformVec3(name.c_str(), value);
                }

                // ------------------------------------------------------------
                // Upload vec4 uniforms
                // ------------------------------------------------------------
                for (const auto &pair : material.getVec4s()) {
                    const std::string &name = pair.first;
                    const glm::vec4 &value = pair.second;

                    backend.setUniformVec4(name.c_str(), value);
                }
            }

            void drawEntity(std::shared_ptr<Scene::IEntity> entity, const ResourceRegistry &resources,
                            IRenderBackend &backend) {
                if (!entity)
                    return;

                // Fetch mesh + material
                auto mesh = resources.getMesh(entity->getMeshId());
                auto material = resources.getMaterial(entity->getMaterialId());

                if (!mesh || !material)
                    return;

                // Shader comes directly from material
                auto shader = material->getShaderName();
                backend.setShader(resources.getShader(shader));

                // Temporary hardcoded directional light or point light:
                // (Replace with values from RenderSettings later)
                backend.setUniformVec3("uLightPos", glm::vec3(4, 8, 4));
                backend.setUniformVec3("uLightColor", glm::vec3(300, 300, 300));

                // Upload material properties (albedo, roughness, metallic, etc)
                uploadMaterialProperties(backend, *material);

                // Build model matrix
                glm::mat4 model = glm::translate(glm::mat4(1.0f), entity->getPosition());
                model = glm::scale(model, entity->getScale());

                backend.drawMesh(*mesh, model);
            }
        };

    } // namespace Render
} // namespace Arche