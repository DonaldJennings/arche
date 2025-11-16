#pragma once
#include <ISubsystem.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <array>
#include <memory>

#include <Camera.h>
#include <IEntity.h>

// Ensure correct type usage from the Arche::Render namespace
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"

namespace Arche {
    namespace Render {

        /**
         * @brief Abstract interface for graphics API backends.
         * 
         * IRenderBackend provides a platform-agnostic interface for 3D rendering
         * operations. Concrete implementations (like OpenGLBackend) handle the
         * actual graphics API calls for their respective platforms.
         * 
         * The backend manages the rendering context, frame lifecycle, shader/material
         * binding, mesh drawing, and advanced features like shadow mapping and
         * debug visualization.
         * 
         * @note All rendering must occur between beginFrame() and endFrame() calls.
         */
        class IRenderBackend {
          public:
            /**
             * @brief Virtual destructor for proper cleanup.
             */
            virtual ~IRenderBackend() = default;

            /**
             * @brief Initialize the rendering backend.
             * 
             * Sets up the graphics context, creates default resources, and
             * prepares the backend for rendering.
             * 
             * @throws std::runtime_error if initialization fails
             */
            virtual void initialise() = 0;
            
            /**
             * @brief Shutdown the rendering backend.
             * 
             * Releases all graphics resources and cleans up the context.
             * Must not throw exceptions.
             */
            virtual void shutdown() noexcept = 0;
            
            /**
             * @brief Handle viewport resize.
             * 
             * Updates internal buffers and framebuffers to match the new size.
             * 
             * @param newSize New viewport dimensions in pixels
             */
            virtual void resize(glm::ivec2 newSize) = 0;

            /**
             * @brief Begin rendering a new frame.
             * 
             * Clears buffers and prepares for rendering. Must be called before
             * any draw operations.
             */
            virtual void beginFrame() = 0;
            
            /**
             * @brief Finish rendering the current frame.
             * 
             * Finalizes rendering and presents the frame. Must be called after
             * all draw operations are complete.
             */
            virtual void endFrame() = 0;

            /**
             * @brief Set the view and projection matrices.
             * 
             * These matrices transform from world space to clip space and are
             * used by all subsequent draw calls.
             * 
             * @param view View matrix (world to camera space)
             * @param projection Projection matrix (camera to clip space)
             */
            virtual void setViewProjection(const glm::mat4 &view, const glm::mat4 &projection) = 0;

            /**
             * @brief Bind a shader program for rendering.
             * @param shader Shader to use for subsequent draws
             */
            virtual void setShader(std::shared_ptr<Arche::Render::Shader> shader) = 0;
            
            /**
             * @brief Apply a material's properties to the current shader.
             * @param material Material containing texture and property data
             */
            virtual void setMaterial(const Arche::Render::Material &material) = 0;
            
            /**
             * @brief Draw a mesh with the given transformation.
             * 
             * @param mesh Mesh geometry to draw
             * @param model Model matrix (object to world space)
             */
            virtual void drawMesh(const Arche::Render::Mesh &mesh, const glm::mat4 &model) = 0;
            
            /**
             * @brief Set a 4x4 matrix uniform in the current shader.
             * @param name Uniform variable name
             * @param value Matrix value
             */
            virtual void setUniformMat4(const std::string &name, const glm::mat4 &value) = 0;
            
            /**
             * @brief Set a vec4 uniform in the current shader.
             * @param name Uniform variable name
             * @param value Vector value
             */
            virtual void setUniformVec4(const std::string &name, const glm::vec4 &value) = 0;
            
            /**
             * @brief Set a float uniform in the current shader.
             * @param name Uniform variable name
             * @param value Float value
             */
            virtual void setUniform1f(const std::string &name, float value) = 0;
            
            /**
             * @brief Set an integer uniform in the current shader.
             * @param name Uniform variable name
             * @param value Integer value
             */
            virtual void setUniform1i(const std::string &name, int value) = 0;
            
            /**
             * @brief Set a vec3 uniform in the current shader.
             * @param name Uniform variable name
             * @param value Vector value
             */
            virtual void setUniformVec3(const std::string &name, const glm::vec3 &value) = 0;
            
            /**
             * @brief Get the OpenGL texture ID of the main render target.
             * 
             * Used for displaying the rendered image in ImGui or other UI systems.
             * 
             * @return Texture ID (platform-specific)
             */
            virtual unsigned int getRenderTextureID() const = 0;

            /**
             * @brief Bind a texture to a sampler slot.
             * 
             * @param name Sampler uniform name in the shader
             * @param textureID Platform-specific texture identifier
             * @param slot Texture unit slot (0-31)
             */
            virtual void bindTexture(const std::string &name, unsigned int textureID, int slot) = 0;

            /**
             * @brief Initialize shadow mapping resources.
             * @param resolution Shadow map texture resolution (width and height)
             */
            virtual void initialiseShadowResources(int resolution) = 0;

            /**
             * @brief Begin shadow map rendering pass.
             * 
             * Switches to shadow framebuffer and prepares for depth-only rendering.
             */
            virtual void beginShadowPass() = 0;
            
            /**
             * @brief End shadow map rendering pass.
             * 
             * Restores the main framebuffer.
             */
            virtual void endShadowPass() = 0;
            
            /**
             * @brief Get the shadow map texture ID.
             * @return Texture ID of the shadow depth map
             */
            virtual unsigned int getShadowMapTextureID() const = 0;
            
            /**
             * @brief Draw a mesh with depth-only rendering.
             * 
             * Used for shadow map generation. Only writes to depth buffer.
             * 
             * @param mesh Mesh to draw
             * @param model Model transformation matrix
             */
            virtual void drawMeshDepthOnly(const Arche::Render::Mesh &mesh, const glm::mat4 &model) = 0;

            /**
             * @brief Enable or disable depth buffer writes.
             * @param enabled true to allow depth writes, false to make read-only
             */
            virtual void setDepthMask(bool enabled) = 0;
            
            /**
             * @brief Enable or disable wireframe rendering mode.
             * @param enabled true for wireframe, false for solid
             */
            virtual void setWireframe(bool enabled) = 0;
            
            /**
             * @brief Begin debug line drawing batch.
             * 
             * Prepares for drawing debug visualization lines.
             */
            virtual void beginDebugLines() = 0;
            
            /**
             * @brief Draw a debug line.
             * 
             * @param start Line start point in world space
             * @param end Line end point in world space
             * @param color Line color (RGB)
             */
            virtual void drawDebugLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color) = 0;
            
            /**
             * @brief Finish debug line drawing batch.
             * 
             * Renders all accumulated debug lines.
             */
            virtual void endDebugLines() = 0;
        };

    } // namespace Render
} // namespace Arche