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

        class IRenderBackend {
          public:
            virtual ~IRenderBackend() = default;

            virtual void initialise() = 0;
            virtual void shutdown() noexcept = 0;
            virtual void resize(glm::ivec2 newSize) = 0;

            virtual void beginFrame() = 0;
            virtual void endFrame() = 0;

            virtual void setViewProjection(const glm::mat4 &view, const glm::mat4 &projection) = 0;

            virtual void setShader(std::shared_ptr<Arche::Render::Shader> shader) = 0;
            virtual void setMaterial(const Arche::Render::Material &material) = 0;
            virtual void drawMesh(const Arche::Render::Mesh &mesh, const glm::mat4 &model) = 0;
            virtual void setUniformMat4(const std::string &name, const glm::mat4 &value) = 0;
            virtual void setUniformVec4(const std::string &name, const glm::vec4 &value) = 0;
            virtual void setUniform1f(const std::string &name, float value) = 0;
            virtual void setUniformVec3(const std::string &name, const glm::vec3 &value) = 0;
            virtual unsigned int getRenderTextureID() const = 0;

            virtual void bindTexture(const std::string &name, unsigned int textureID, int slot) = 0;

            virtual void beginShadowPass() = 0;
            virtual void endShadowPass() = 0;
            virtual unsigned int getShadowMapTextureID() const = 0;
            virtual void drawMeshDepthOnly(const Arche::Render::Mesh &mesh, const glm::mat4 &model) = 0;

            virtual void setDepthMask(bool enabled) = 0;
            virtual void setWireframe(bool enabled) = 0;
            virtual void beginDebugLines() = 0;
            virtual void drawDebugLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color) = 0;
            virtual void endDebugLines() = 0;
        };

    } // namespace Render
} // namespace Arche