#pragma once
#include <ISubsystem.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <array>
#include <memory>

#include <Camera.h>
#include <IEntity.h>

class Shader;
class Material;
class Mesh;

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

            virtual void setShader(const std::shared_ptr<Shader> shader) = 0;
            virtual void setMaterial(const Material &material) = 0;
            virtual void drawMesh(const Mesh &mesh, const glm::mat4 &model) = 0;
            virtual unsigned int getRenderTextureID() const = 0;

        };

    } // namespace Render
} // namespace Arche