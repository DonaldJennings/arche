#include "OpenGLBackend.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

// Minimal point sprite shader sources (adapted from Renderer.cpp)
static const char *kPointVS = R"glsl(
#version 460 core
layout(location = 0) in vec3 inPosition;
uniform mat4 uView;
uniform mat4 uProj;
uniform float uPointSize;
void main() {
    gl_Position = uProj * uView * vec4(inPosition, 1.0);
    gl_PointSize = uPointSize;
}
)glsl";

static const char *kPointFS = R"glsl(
#version 460 core
out vec4 fragColor;
uniform vec3 uPointColor;
void main() {
    float r = dot(gl_PointCoord - vec2(0.5), gl_PointCoord - vec2(0.5));
    if (r > 0.25) discard;
    fragColor = vec4(uPointColor, 1.0);
}
)glsl";

namespace {
    GLuint compileShader(GLenum type, const char *src) {
        GLuint sh = glCreateShader(type);
        glShaderSource(sh, 1, &src, nullptr);
        glCompileShader(sh);
        GLint ok = GL_FALSE;
        glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[1024];
            glGetShaderInfoLog(sh, sizeof log, nullptr, log);
            glDeleteShader(sh);
            return 0;
        }
        return sh;
    }

    GLuint linkProgram(GLuint vs, GLuint fs) {
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);
        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[1024];
            glGetProgramInfoLog(prog, sizeof log, nullptr, log);
            glDeleteProgram(prog);
            return 0;
        }
        return prog;
    }
}

using namespace Arche::Render;

OpenGLBackend::OpenGLBackend() = default;

void OpenGLBackend::initialise() {
    // We assume an OpenGL context is already current (created by the app / ImGui layer)
    // Global state enabling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create FBO and color/depth attachments
    initialiseFrameBuffer();

    // Create minimal point pipeline
    ensurePointPipeline();

    // Default cached matrices: identity
    for (int i = 0; i < 16; ++i) {
        m_viewF[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        m_projF[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

void OpenGLBackend::shutdown() noexcept {
    // FBO resources
    if (m_colorTexture) { glDeleteTextures(1, &m_colorTexture); m_colorTexture = 0; }
    if (m_depthStencilRbo) { glDeleteRenderbuffers(1, &m_depthStencilRbo); m_depthStencilRbo = 0; }
    if (m_frameBuffer) { glDeleteFramebuffers(1, &m_frameBuffer); m_frameBuffer = 0; }

    // Pipeline resources
    if (m_pointVBO) { glDeleteBuffers(1, &m_pointVBO); m_pointVBO = 0; }
    if (m_pointVAO) { glDeleteVertexArrays(1, &m_pointVAO); m_pointVAO = 0; }
    if (m_pointProgram) { glDeleteProgram(m_pointProgram); m_pointProgram = 0; }
}

void OpenGLBackend::resize(glm::ivec2 newSize) {
    if (newSize.x <= 0 || newSize.y <= 0) return;
    m_Backbuffer = newSize;

    // Recreate FBO at new size
    if (m_frameBuffer) { glDeleteFramebuffers(1, &m_frameBuffer); m_frameBuffer = 0; }
    if (m_colorTexture) { glDeleteTextures(1, &m_colorTexture); m_colorTexture = 0; }
    if (m_depthStencilRbo) { glDeleteRenderbuffers(1, &m_depthStencilRbo); m_depthStencilRbo = 0; }

    initialiseFrameBuffer();
}

void OpenGLBackend::beginFrame() {
    // Bind offscreen FBO and clear
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glViewport(0, 0, m_Backbuffer.x, m_Backbuffer.y);
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_pointProgram) {
        glUseProgram(m_pointProgram);
        // Upload cached matrices once per frame
        GLint locView = glGetUniformLocation(m_pointProgram, "uView");
        GLint locProj = glGetUniformLocation(m_pointProgram, "uProj");
        if (locView >= 0) glUniformMatrix4fv(locView, 1, GL_FALSE, m_viewF);
        if (locProj >= 0) glUniformMatrix4fv(locProj, 1, GL_FALSE, m_projF);
        // Set default color
        GLint locColor = glGetUniformLocation(m_pointProgram, "uPointColor");
        if (locColor >= 0) glUniform3f(locColor, 0.9f, 0.6f, 0.2f);
    }
}

void OpenGLBackend::endFrame() {
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLBackend::setViewProjection(const glm::mat4 &view, const glm::mat4 &projection) {
    const float *vptr = glm::value_ptr(view);
    const float *pptr = glm::value_ptr(projection);
    for (int i = 0; i < 16; ++i) {
        m_viewF[i] = vptr[i];
        m_projF[i] = pptr[i];
    }
}

void OpenGLBackend::setShader(const std::shared_ptr<Shader> /*shader*/) {
    // Placeholder: backend uses its own minimal shader for point rendering.
}

void OpenGLBackend::setMaterial(const Material & /*material*/) {
    // Placeholder: set uniforms/textures from your material system here.
}

void OpenGLBackend::drawMesh(const Mesh & /*mesh*/, const glm::mat4 &model) {
    if (!m_pointProgram) return;

    // Minimal point draw at model origin, derive point-size from model scale X
    glm::vec3 p = glm::vec3(model * glm::vec4(0, 0, 0, 1));
    float pos[3] = { p.x, p.y, p.z };

    float sx = glm::length(glm::vec3(model[0]));
    float pointSize = std::max(1.0f, sx * 2.0f);

    GLint locPointSize = glGetUniformLocation(m_pointProgram, "uPointSize");
    if (locPointSize >= 0) glUniform1f(locPointSize, pointSize);

    glBindBuffer(GL_ARRAY_BUFFER, m_pointVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_DYNAMIC_DRAW);

    glBindVertexArray(m_pointVAO);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    glDrawArrays(GL_POINTS, 0, 1);

    glBindVertexArray(0);
    glDisable(GL_PROGRAM_POINT_SIZE);
}

bool OpenGLBackend::initialiseFrameBuffer() {
    // Texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Backbuffer.x, m_Backbuffer.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // FBO
    glGenFramebuffers(1, &m_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    // Depth-stencil RBO
    glGenRenderbuffers(1, &m_depthStencilRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Backbuffer.x, m_Backbuffer.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilRbo);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    // Clear once so the UI samples valid pixels
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

bool OpenGLBackend::ensurePointPipeline() {
    if (m_pointProgram) return true;

    GLuint vs = compileShader(GL_VERTEX_SHADER, kPointVS);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kPointFS);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }

    m_pointProgram = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!m_pointProgram) return false;

    glGenVertexArrays(1, &m_pointVAO);
    glGenBuffers(1, &m_pointVBO);

    glBindVertexArray(m_pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_pointVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    return true;
}
