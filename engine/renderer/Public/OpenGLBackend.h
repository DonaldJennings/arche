#pragma once
#include <array>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include <IRenderBackend.h>

struct GLFWwindow;

namespace Arche { namespace Core { class LoggingService; } }

namespace Arche {
    namespace Render {
        class OpenGLBackend : public IRenderBackend {
          public:
            OpenGLBackend();

            ~OpenGLBackend() noexcept override { shutdown(); }
            void initialise() override;
            void shutdown() noexcept override;
            void resize(glm::ivec2 newSize) override;
            void beginFrame() override;
            void endFrame() override;
            void setViewProjection(const glm::mat4 &view, const glm::mat4 &projection) override;
            void setShader(const std::shared_ptr<Shader> shader) override;
            void setMaterial(const Material &material) override;
            void drawMesh(const Mesh &mesh, const glm::mat4 &model) override;
            unsigned int getRenderTextureID() const override { return m_colorTexture; }
          private:
            // Backbuffer and cached matrices
            glm::ivec2 m_Backbuffer{800, 600};
            glm::mat4 m_lastViewMatrix{1.0f};
            glm::mat4 m_lastProjectionMatrix{1.0f};

            // Optional window/context pointer if needed by the app (we assume context is current)
            GLFWwindow *window = nullptr;
            std::shared_ptr<Arche::Core::LoggingService> mLogger;

            // GL resources for offscreen rendering
            unsigned int m_frameBuffer = 0;
            unsigned int m_colorTexture = 0;
            unsigned int m_depthStencilRbo = 0;

            // Minimal point rendering pipeline
            unsigned int m_pointProgram = 0;
            unsigned int m_pointVAO = 0;
            unsigned int m_pointVBO = 0;

            // Cached view/projection for uniform uploads
            float m_viewF[16]{};
            float m_projF[16]{};

            // Clear color
            glm::vec4 m_clearColor{0.1f, 0.12f, 0.15f, 1.0f};

            // Helpers
            bool initialiseFrameBuffer();
            bool ensurePointPipeline();
        };
    } // namespace Render
} // namespace Arche