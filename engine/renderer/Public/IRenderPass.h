#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "Shader.h"
#include "IRenderBackend.h"
#include "ResourceRegistry.h"
#include "GlobalSettings.h"
#include <WorldSystem.h>

namespace Arche {
    namespace Render {

        /**
         * @brief Shadow mapping configuration and state.
         * 
         * Contains all settings and resources needed for shadow rendering,
         * including the shadow map texture, light space matrix, and resolution.
         */
        struct ShadowSettings
        {
            unsigned int shadowMapID{0};        ///< Shadow depth texture ID
            glm::mat4 lightSpaceMatrix{1.0f};  ///< Transform to light's perspective
            int resolution{2048};               ///< Shadow map resolution
            bool enabled{false};                ///< Whether shadows are active
        };
        
        /**
         * @brief Global settings passed to render passes.
         * 
         * Aggregates shadow settings and global render configuration that
         * all render passes may need access to.
         */
        class RenderPassSettings
        {
          public:
            RenderPassSettings() = default;
            ShadowSettings shadowSettings;          ///< Shadow configuration
            Core::RenderSettings globalSettings;    ///< Global render settings

          private:
        };

        /**
         * @brief View of the scene for rendering.
         * 
         * Contains camera matrices, position, and lists of objects to render.
         * Objects are separated by transparency for correct rendering order.
         */
        struct RenderView {
            glm::mat4 viewMatrix;           ///< Camera view transform
            glm::mat4 projectionMatrix;     ///< Perspective projection

            glm::vec3 cameraPosition;       ///< Camera world position

            std::vector<std::shared_ptr<Scene::IEntity>> opaqueObjects;      ///< Solid objects
            std::vector<std::shared_ptr<Scene::IEntity>> transparentObjects; ///< Transparent objects
        };


        /**
         * @brief Abstract interface for rendering passes.
         * 
         * A render pass represents one stage of the rendering pipeline, such as
         * shadow map generation, geometry rendering, or skybox drawing. Multiple
         * passes are executed in sequence to produce the final image.
         * 
         * Render passes have complete control over what and how they render,
         * allowing for complex multi-pass rendering techniques.
         */
        class IRenderPass
        {
          public:
            /**
             * @brief Virtual destructor for proper cleanup.
             */
            virtual ~IRenderPass() = default;
            
            /**
             * @brief Initialize the render pass.
             * 
             * Called once during rendering system initialization. Use this to
             * create pass-specific resources, load shaders, etc.
             * 
             * @param backend Reference to the rendering backend
             */
            virtual void initialise(IRenderBackend &backend) = 0;
            
            /**
             * @brief Execute the render pass.
             * 
             * Performs the actual rendering work for this pass. May modify
             * backend state, bind shaders, and issue draw calls.
             * 
             * @param view Scene view with camera and objects to render
             * @param backend Reference to the rendering backend
             * @param camera Camera for this frame
             * @param resources Resource registry for meshes/materials/shaders
             * @param settings Global and shadow rendering settings
             */
            virtual void render(const RenderView &view, IRenderBackend &backend, const Camera &camera,
                                ResourceRegistry &resources, RenderPassSettings &settings) = 0;
            
            /**
             * @brief Shutdown the render pass.
             * 
             * Called during rendering system shutdown. Release any resources
             * allocated by this pass.
             * 
             * @param backend Reference to the rendering backend
             */
            virtual void shutdown(IRenderBackend &backend) = 0;
        };

    } // namespace Render
} // namespace Arche