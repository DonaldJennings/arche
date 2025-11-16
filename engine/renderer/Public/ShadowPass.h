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

    /**
     * @brief Render pass for shadow map generation.
     * 
     * ShadowPass renders the scene from the light's perspective into a
     * depth texture (shadow map). This shadow map is then used by later
     * passes to determine which areas are in shadow.
     * 
     * The pass uses a depth-only rendering mode for efficiency, as only
     * depth information is needed for shadow mapping.
     */
    class ShadowPass : public IRenderPass {
      public:
        /**
         * @brief Construct a shadow pass with specified resolution.
         * 
         * @param resolution Shadow map texture size (width and height)
         */
        ShadowPass(int resolution = 2048) : m_resolution(resolution) {}

        /**
         * @brief Initialize shadow mapping resources.
         * 
         * Creates the shadow map framebuffer and depth texture.
         * 
         * @param backend Reference to the rendering backend
         */
        void initialise(IRenderBackend &backend) override;

        /**
         * @brief Render shadow map from light's perspective.
         * 
         * Computes light space matrix, renders all scene objects depth-only
         * into the shadow map, and stores the results in RenderPassSettings
         * for use by later passes.
         * 
         * @param view Scene view with objects to render
         * @param backend Reference to the rendering backend
         * @param camera Camera for this frame (not used for shadow rendering)
         * @param resources Resource registry for meshes/materials/shaders
         * @param settings Global rendering settings (shadow map stored here)
         */
        void render(const RenderView &view, IRenderBackend &backend, const Camera &camera, ResourceRegistry &resources,
                    RenderPassSettings &settings) override;

        /**
         * @brief Shutdown the shadow pass.
         * 
         * Currently a no-op as resources are managed by the backend.
         * 
         * @param backend Reference to the rendering backend
         */
        void shutdown(IRenderBackend &backend) override {}

      private:
        int m_resolution;                       ///< Shadow map resolution
        glm::mat4 m_lightSpaceMatrix{1.0f};    ///< Transform to light space
        unsigned int m_shadowMapID{0};          ///< Shadow depth texture ID
        bool m_initialised{false};              ///< Initialization state
    };

} // namespace Arche::Render
