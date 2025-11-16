#pragma once
#include <glm/glm.hpp>
#include <memory>

#include "Camera.h"
#include "IRenderBackend.h"
#include "IRenderPass.h"
#include "Material.h"
#include "Mesh.h"
#include "ResourceRegistry.h"
#include "GlobalSettings.h"
#include "Shader.h"

namespace Arche::Render {

    /**
     * @brief Render pass for procedural sky rendering.
     * 
     * SkyPass renders a dynamic sky background using a procedural shader.
     * The sky color and appearance are computed based on view direction,
     * sun position, and atmospheric scattering approximations.
     * 
     * This pass typically runs early (after shadow pass) to fill the background
     * before rendering geometry.
     */
    class SkyPass : public IRenderPass {
      public:
        SkyPass() = default;

        /**
         * @brief Initialize the sky pass.
         * 
         * Sets up sky rendering resources and shaders.
         * 
         * @param backend Reference to the rendering backend
         */
        void initialise(IRenderBackend &backend) override;

        /**
         * @brief Render the procedural sky.
         * 
         * Draws a full-screen quad with a sky shader that computes
         * color based on view direction and sun parameters.
         * 
         * @param view Scene view with camera information
         * @param backend Reference to the rendering backend
         * @param camera Camera for this frame
         * @param resources Resource registry for meshes/materials/shaders
         * @param settings Global rendering settings
         */
        void render(const RenderView &view, IRenderBackend &backend, const Camera &camera, ResourceRegistry &resources,
                    RenderPassSettings &settings) override;

        /**
         * @brief Shutdown the sky pass.
         * 
         * Releases sky rendering resources.
         * 
         * @param backend Reference to the rendering backend
         */
        void shutdown(IRenderBackend &backend) override;

      private:
        /**
         * @brief Upload sky shader uniforms.
         * 
         * @param backend Reference to the rendering backend
         * @param inverseViewProjection Inverse view-projection for ray direction
         */
        void uploaderUniformProperties(IRenderBackend &backend, glm::mat4 inverseViewProjection);

        glm::vec3 m_sunDirection{0.3f, 0.8f, 0.2f};  ///< Sun direction vector
        glm::vec3 m_sunColor{5.0f, 5.0f, 4.8f};      ///< Sun color/intensity
    };

} // namespace Arche::Render
