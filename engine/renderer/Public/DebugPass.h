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

    /**
     * @brief Render pass for debug visualization.
     * 
     * DebugPass draws debug overlays like grid lines, coordinate axes,
     * and other visual aids to help developers understand the 3D space.
     * This pass typically runs last to ensure debug visualizations appear
     * on top of the scene.
     * 
     * Debug elements are controlled by global render settings and can be
     * toggled on/off from the editor UI.
     */
    class DebugPass : public IRenderPass {
      public:
        DebugPass() = default;
        
        /**
         * @brief Initialize the debug pass.
         * 
         * Currently a no-op as the pass doesn't need initialization.
         * 
         * @param backend Reference to the rendering backend
         */
        void initialise(IRenderBackend &backend) override {};
        
        /**
         * @brief Render debug overlays.
         * 
         * Draws grid, axes, and other debug visualizations based on
         * settings. Only draws elements that are enabled in settings.
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
         * @brief Shutdown the debug pass.
         * 
         * Currently a no-op as the pass doesn't allocate resources.
         * 
         * @param backend Reference to the rendering backend
         */
        void shutdown(IRenderBackend &backend) override {};

      private:
        /**
         * @brief Draw the ground grid.
         * 
         * @param backend Reference to the rendering backend
         * @param cameraPosition Camera position for grid centering
         * @param resources Resource registry
         */
        void drawGrid(IRenderBackend &backend, const glm::vec3 &cameraPosition, ResourceRegistry &resources);
    };

} // namespace Arche::Render
