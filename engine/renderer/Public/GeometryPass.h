#pragma once
#include "IRenderPass.h"
#include "ResourceRegistry.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Arche {
    namespace Render {

        /**
         * @brief Main render pass for drawing scene geometry.
         * 
         * GeometryPass is responsible for rendering all opaque objects in the scene
         * with full lighting and materials. It applies the camera view/projection,
         * sets up lighting parameters, configures shadow mapping, and draws each
         * entity with its associated mesh and material.
         * 
         * This is typically the main rendering pass that produces the visible
         * scene content. It runs after shadow and sky passes.
         */
        class GeometryPass : public IRenderPass {
          public:
            /**
             * @brief Initialize the geometry pass.
             * 
             * Sets up any resources needed for geometry rendering.
             * 
             * @param backend Reference to the rendering backend
             */
            void initialise(IRenderBackend &backend) override {
                // Initialization code for the geometry pass
            }

            /**
             * @brief Render all opaque geometry with lighting.
             * 
             * Applies camera matrices, then iterates through all opaque objects,
             * drawing each with its material and applying global lighting.
             * 
             * @param view Scene view with camera and objects to render
             * @param backend Reference to the rendering backend
             * @param camera Camera for this frame
             * @param resources Resource registry for meshes/materials/shaders
             * @param settings Global rendering and shadow settings
             */
            void render(const RenderView &view, IRenderBackend &backend, const Camera &camera,
                        ResourceRegistry &resources, RenderPassSettings &settings) override {

                // Apply camera matrices globally
                backend.setViewProjection(view.viewMatrix, view.projectionMatrix);

                // Opaque geometry
                for (const auto &obj : view.opaqueObjects) {
                    drawObject(obj, resources, backend, settings, view.cameraPosition);
                }
            }

            /**
             * @brief Shutdown the geometry pass.
             * 
             * Releases any resources allocated by the geometry pass.
             * 
             * @param backend Reference to the rendering backend
             */
            void shutdown(IRenderBackend &backend) override {
                // Cleanup code for the geometry pass
            }

          private:
            /**
             * @brief Upload custom material properties to shader uniforms.
             * 
             * Applies all float, vec3, and vec4 parameters defined in the
             * material to the currently bound shader.
             * 
             * @param backend Reference to the rendering backend
             * @param material Material containing custom properties
             */
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
            }

            /**
             * @brief Draw a single entity with lighting and shadows.
             * 
             * Fetches the entity's mesh and material, binds the appropriate shader,
             * sets up lighting and shadow uniforms, computes the model matrix,
             * and issues the draw call.
             * 
             * @param entity Entity to render
             * @param resources Resource registry for lookups
             * @param backend Reference to the rendering backend
             * @param settings Rendering settings (lighting, shadows)
             * @param cameraPos Camera position for lighting calculations
             */
            void drawObject(const RenderObject &obj, const ResourceRegistry &resources,
                            IRenderBackend &backend, RenderPassSettings &settings, glm::vec3 cameraPos) {
                auto mesh     = resources.getMesh(obj.meshId);
                auto material = resources.getMaterial(obj.materialId);
                if (!mesh || !material) return;

                backend.setShader(resources.getShader(material->getShaderName()));

                backend.setMaterial(*material);

                backend.setUniformVec3("uCameraPos", cameraPos);
                backend.setUniformVec3("uLightDir", -settings.globalSettings.directionalLightDirection);
                backend.setUniformVec3("uLightColor", settings.globalSettings.directionalLightColor);

                backend.setUniform1i("uHasShadowMap", settings.shadowSettings.enabled ? 1 : 0);

                if (settings.shadowSettings.enabled) {
                    backend.setUniformMat4("uLightSpaceMatrix", settings.shadowSettings.lightSpaceMatrix);
                    backend.bindTexture("uShadowMap", settings.shadowSettings.shadowMapID, 5);
                }

                uploadMaterialProperties(backend, *material);

                backend.drawMesh(*mesh, obj.transform);
            }
        };

    } // namespace Render
} // namespace Arche