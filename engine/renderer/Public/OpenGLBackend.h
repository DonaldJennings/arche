#pragma once
#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <IRenderBackend.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>

struct GLFWwindow;

namespace Arche {
    namespace Core {
        class LoggingService;
    }
} // namespace Arche

namespace Arche {
    namespace Render {

        struct GLShaderProgram {
            GLuint programID = 0;
            std::unordered_map<std::string, GLint> uniformLocations;
        };

        struct GLMesh {
            GLuint vertexArrayObject = 0;
            GLuint vertexBufferObject = 0;
            GLuint elementBufferObject = 0;
            GLsizei indexCount = 0;
            GLsizei vertexCount = 0;
        };

        struct GLTexture {
            GLuint textureID = 0;
        };

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
            void setShader(std::shared_ptr<Shader> shader) override;
            void setMaterial(const Material &material) override;
            void setLogger(std::shared_ptr<Core::LoggingService> loggerIn) { mLogger = loggerIn; };
            void drawMesh(const Mesh &mesh, const glm::mat4 &model) override;
            unsigned int getRenderTextureID() const override { return m_colorTexture; }
            void bindTexture(const std::string &name, unsigned int textureID, int slot) override;

            void initialiseShadowResources(int resolution) override;
            void beginShadowPass() override;
            void endShadowPass() override;
            unsigned int getShadowMapTextureID() const override { return m_shadowMapTexture; }
            void drawMeshDepthOnly(const Mesh &mesh, const glm::mat4 &model) override;

            void setUniformMat4(const std::string &name, const glm::mat4 &value) override;
            void setUniformVec4(const std::string &name, const glm::vec4 &value) override;
            void setUniform1f(const std::string &name, float value) override;
            void setUniform1i(const std::string &name, int value) override;
            void setUniformVec3(const std::string &name, const glm::vec3 &value) override;
            void setDepthMask(bool enabled) override;
            void setWireframe(bool enabled) override;
            void beginDebugLines() override;
            void drawDebugLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color) override;
            void endDebugLines() override;
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

            // Shadow mapping resources
            GLuint m_shadowFrameBuffer = 0;
            GLuint m_shadowMapTexture = 0;
            int m_shadowMapResolution = 2048;

            GLShaderProgram *m_shadowProgram = nullptr;

            std::unordered_map<std::string, GLShaderProgram> m_shaders;
            std::unordered_map<const Mesh *, GLMesh> m_meshes;

            GLShaderProgram *m_currentShader = nullptr;

            // Return references to cache-stored objects (avoid dangling temporaries)
            GLShaderProgram &getOrCreateShaderProgram(const std::shared_ptr<Shader> &shader);
            GLMesh &getOrCreateGLMesh(const Mesh &mesh);

            GLint getUniformLocation(GLShaderProgram &program, const std::string &name);

            // Cached view/projection for uniform uploads
            float m_viewF[16]{};
            float m_projF[16]{};

            // Clear color
            glm::vec4 m_clearColor{0.1f, 0.12f, 0.15f, 1.0f};

            // Helpers
            bool initialiseFrameBuffer();

            // Debug line buffered draw state
            GLuint m_debugLinesVao = 0;
            GLuint m_debugLinesVbo = 0;
            std::vector<float> m_debugLinesData; // [x,y,z,r,g,b] per-vertex
        };
    } // namespace Render
} // namespace Arche