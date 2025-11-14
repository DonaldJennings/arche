#include <glad/glad.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

#include "OpenGLBackend.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

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
            std::cerr << "[OpenGL] Shader compilation failed:\n" << log << "\n";
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
            std::cerr << "[OpenGL] Shader linking failed:\n" << log << "\n";
            glDeleteProgram(prog);
            return 0;
        }
        return prog;
    }

    static std::string prependDefines(const std::string &source,
                                      const std::unordered_map<std::string, std::string> &defines) {
        if (defines.empty())
            return source;

        // find the version line
        size_t pos = source.find("#version");
        if (pos == std::string::npos) {
            // no version? prepend everything as fallback
            std::string header;
            for (auto &[k, v] : defines)
                header += "#define " + k + " " + v + "\n";
            header += "\n";
            return header + source;
        }

        // find end of line after the #version directive
        size_t lineEnd = source.find('\n', pos);
        if (lineEnd == std::string::npos)
            lineEnd = source.size();

        // build new output
        std::string output;
        output.reserve(source.size() + defines.size() * 32);

        // copy up to end of version line
        output = source.substr(0, lineEnd + 1);

        // add defines AFTER the version line
        for (auto &[k, v] : defines)
            output += "#define " + k + " " + v + "\n";

        // append rest of shader code
        output += source.substr(lineEnd + 1);

        return output;
    }
} // namespace

using namespace Arche::Render;

OpenGLBackend::OpenGLBackend() = default;

GLint OpenGLBackend::getUniformLocation(GLShaderProgram &program, const std::string &name) {
    auto it = program.uniformLocations.find(name);
    if (it != program.uniformLocations.end()) {
        return it->second;
    }
    GLint loc = glGetUniformLocation(program.programID, name.c_str());
    program.uniformLocations[name] = loc;
    return loc;
}

GLShaderProgram &OpenGLBackend::getOrCreateShaderProgram(const std::shared_ptr<Shader> &shader) {
    auto it = m_shaders.find(shader->getName());
    if (it != m_shaders.end()) {
        return it->second;
    }

    // Build final GLSL sources with defines injected
    const Shader::Sources &srcs{shader->getSources()};
    std::string vertSrc{prependDefines(srcs.vertexGLSL, shader->getDefines())};
    std::string fragSrc{prependDefines(srcs.fragmentGLSL, shader->getDefines())};

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc.c_str());
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str());

    if (!vs || !fs) {
        if (vs)
            glDeleteShader(vs);
        if (fs)
            glDeleteShader(fs);
        // Insert a dummy program to avoid repeated compile attempts
        auto [insIt, _] = m_shaders.emplace(shader->getName(), GLShaderProgram{});
        return insIt->second;
    }

    GLuint programID{linkProgram(vs, fs)};
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLShaderProgram program;
    program.programID = programID;

    auto [iteratorToProgram, inserted] = m_shaders.emplace(shader->getName(), std::move(program));
    return iteratorToProgram->second;
}

void OpenGLBackend::initialise() {
    // We assume an OpenGL context is already current (created by the app / ImGui layer)
    // Global state enabling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Reduce chances of rehash invalidating references
    m_shaders.reserve(8);
    m_meshes.reserve(64);

    // Create FBO and color/depth attachments
    initialiseFrameBuffer();

    initialiseShadowResources(m_shadowMapResolution);

    // Default cached matrices: identity
    for (int i = 0; i < 16; ++i) {
        m_viewF[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        m_projF[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

void OpenGLBackend::shutdown() noexcept {
    // FBO resources
    if (m_colorTexture) {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }
    if (m_depthStencilRbo) {
        glDeleteRenderbuffers(1, &m_depthStencilRbo);
        m_depthStencilRbo = 0;
    }
    if (m_frameBuffer) {
        glDeleteFramebuffers(1, &m_frameBuffer);
        m_frameBuffer = 0;
    }

    if (m_shadowMapTexture) {
        glDeleteTextures(1, &m_shadowMapTexture);
        m_shadowMapTexture = 0;
    }

    if (m_shadowFrameBuffer) {
        glDeleteFramebuffers(1, &m_shadowFrameBuffer);
        m_shadowFrameBuffer = 0;
    }

    // Mesh resources
    for (auto &[key, glMesh] : m_meshes) {
        if (glMesh.elementBufferObject)
            glDeleteBuffers(1, &glMesh.elementBufferObject);
        if (glMesh.vertexBufferObject)
            glDeleteBuffers(1, &glMesh.vertexBufferObject);
        if (glMesh.vertexArrayObject)
            glDeleteVertexArrays(1, &glMesh.vertexArrayObject);
    }
    m_meshes.clear();

    // Shader programs
    for (auto &[name, glProg] : m_shaders) {
        if (glProg.programID)
            glDeleteProgram(glProg.programID);
    }
    m_shaders.clear();

    m_currentShader = nullptr;
}

void OpenGLBackend::resize(glm::ivec2 newSize) {
    if (newSize.x <= 0 || newSize.y <= 0)
        return;
    m_Backbuffer = newSize;

    // Recreate FBO at new size
    if (m_frameBuffer) {
        glDeleteFramebuffers(1, &m_frameBuffer);
        m_frameBuffer = 0;
    }
    if (m_colorTexture) {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }
    if (m_depthStencilRbo) {
        glDeleteRenderbuffers(1, &m_depthStencilRbo);
        m_depthStencilRbo = 0;
    }

    initialiseFrameBuffer();
}

void OpenGLBackend::beginFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glViewport(0, 0, m_Backbuffer.x, m_Backbuffer.y);
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

void OpenGLBackend::setShader(const std::shared_ptr<Shader> shader) {

    if (!shader) {
        m_currentShader = nullptr;
        glUseProgram(0);
        return;
    }

    GLShaderProgram &prog = getOrCreateShaderProgram(shader);
    m_currentShader = &prog;
    glUseProgram(prog.programID);

    if (m_currentShader) {
        GLint locView = getUniformLocation(*m_currentShader, "uView");
        GLint locProj = getUniformLocation(*m_currentShader, "uProj");
        if (locView >= 0)
            glUniformMatrix4fv(locView, 1, GL_FALSE, m_viewF);
        if (locProj >= 0)
            glUniformMatrix4fv(locProj, 1, GL_FALSE, m_projF);
    }
}

GLMesh &OpenGLBackend::getOrCreateGLMesh(const Mesh &mesh) {
    // First check the cache for the mesh
    auto meshIterator{m_meshes.find(&mesh)};
    if (meshIterator != m_meshes.end()) {
        return meshIterator->second;
    }

    // Otherwise create a mesh in the cache
    GLMesh gl{};
    glGenVertexArrays(1, &gl.vertexArrayObject);
    glGenBuffers(1, &gl.vertexBufferObject);
    glGenBuffers(1, &gl.elementBufferObject);

    glBindVertexArray(gl.vertexArrayObject);

    const auto &vertices{mesh.vertices()};
    const auto &indices{mesh.indices()};

    glBindBuffer(GL_ARRAY_BUFFER, gl.vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Mesh::Vertex)),
                 vertices.empty() ? nullptr : vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl.elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)),
                 indices.empty() ? nullptr : indices.data(), GL_STATIC_DRAW);

    // Vertex layout
    GLsizei stride{static_cast<GLsizei>(sizeof(Mesh::Vertex))};

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offsetof(Mesh::Vertex, position)));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offsetof(Mesh::Vertex, normal)));

    // UV
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offsetof(Mesh::Vertex, uv)));

    glBindVertexArray(0);

    gl.indexCount = static_cast<GLsizei>(indices.size());
    gl.vertexCount = static_cast<GLsizei>(vertices.size());

    auto [iteratorToMesh, inserted] = m_meshes.emplace(&mesh, gl);
    return iteratorToMesh->second;
}

void OpenGLBackend::setMaterial(const Material &material) {
    if (!m_currentShader) {
        return;
    }

    GLint locColor = getUniformLocation(*m_currentShader, "uBaseColor");
    if (locColor >= 0)
        glUniform4fv(locColor, 1, glm::value_ptr(material.getBaseColor()));

    GLint locPoint = getUniformLocation(*m_currentShader, "uPointSize");
    if (locPoint >= 0)
        glUniform1f(locPoint, material.getPointSize());
}

void OpenGLBackend::drawMesh(const Mesh &mesh, const glm::mat4 &model) {
    if (!m_currentShader)
        return;

    GLint locModel{getUniformLocation(*m_currentShader, "uModel")};

    if (locModel >= 0) {
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
    }

    GLMesh &glMesh = getOrCreateGLMesh(mesh);

    glBindVertexArray(glMesh.vertexArrayObject);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (glMesh.indexCount > 0) {
        glDrawElements(GL_TRIANGLES, glMesh.indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, glMesh.vertexCount);
    }
}


void OpenGLBackend::beginShadowPass() {
    if (m_shadowFrameBuffer == 0)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFrameBuffer);
    glViewport(0, 0, m_shadowMapResolution, m_shadowMapResolution);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLBackend::endShadowPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glViewport(0, 0, m_Backbuffer.x, m_Backbuffer.y);
    glUseProgram(0);
}

void OpenGLBackend::initialiseShadowResources(int resolution) {
    if (resolution <= 0)
        resolution = 1024;

    if (m_shadowMapTexture) {
        glDeleteTextures(1, &m_shadowMapTexture);
        m_shadowMapTexture = 0;
    }

    if (m_shadowFrameBuffer) {
        glDeleteFramebuffers(1, &m_shadowFrameBuffer);
        m_shadowFrameBuffer = 0;
    }

    m_shadowMapResolution = resolution;

    glGenFramebuffers(1, &m_shadowFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFrameBuffer);

    glGenTextures(1, &m_shadowMapTexture);
    glBindTexture(GL_TEXTURE_2D, m_shadowMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_shadowMapResolution, m_shadowMapResolution, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float borderColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[OpenGL] Shadow framebuffer incomplete: 0x" << std::hex << status << std::dec << "\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLBackend::drawMeshDepthOnly(const Mesh &mesh, const glm::mat4 &model) {
    if (!m_currentShader->programID)
        return;

    glUseProgram(m_currentShader->programID);

    GLint locModel = getUniformLocation(*m_currentShader, "uModel");
    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

    GLMesh &glMesh = getOrCreateGLMesh(mesh);
    glBindVertexArray(glMesh.vertexArrayObject);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    if (glMesh.indexCount > 0) {
        glDrawElements(GL_TRIANGLES, glMesh.indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, glMesh.vertexCount);
    }

    glBindVertexArray(0);
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

void OpenGLBackend::setUniformMat4(const std::string &name, const glm::mat4 &value) {
    if (!m_currentShader || m_currentShader->programID == 0)
        return;
    GLint loc = getUniformLocation(*m_currentShader, name);
    if (loc >= 0)
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void OpenGLBackend::setUniformVec4(const std::string &name, const glm::vec4 &value) {
    if (!m_currentShader || m_currentShader->programID == 0) {
        return;
    }
    GLint loc = getUniformLocation(*m_currentShader, name);
    if (loc >= 0) {
        glUniform4fv(loc, 1, glm::value_ptr(value));
    }
}

void OpenGLBackend::setUniformVec3(const std::string &name, const glm::vec3 &value) {
    if (!m_currentShader || m_currentShader->programID == 0)
        return;
    GLint loc = getUniformLocation(*m_currentShader, name);
    if (loc >= 0)
        glUniform3fv(loc, 1, glm::value_ptr(value));
}

void OpenGLBackend::setUniform1f(const std::string &name, float value) {
    if (!m_currentShader || m_currentShader->programID == 0)
        return;
    GLint loc = getUniformLocation(*m_currentShader, name);
    if (loc >= 0)
        glUniform1f(loc, value);
}

void OpenGLBackend::setUniform1i(const std::string &name, int value) {
    if (!m_currentShader || m_currentShader->programID == 0)
        return;
    GLint loc = getUniformLocation(*m_currentShader, name);
    if (loc >= 0)
        glUniform1i(loc, value);
}

void OpenGLBackend::setDepthMask(bool enabled) { glDepthMask(enabled ? GL_TRUE : GL_FALSE); }

void OpenGLBackend::setWireframe(bool enabled) {
    if (enabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void OpenGLBackend::beginDebugLines() {
    m_debugLinesData.clear();
}

void OpenGLBackend::drawDebugLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color) {
    // two vertices: pos + color
    m_debugLinesData.reserve(m_debugLinesData.size() + 12);
    // start
    m_debugLinesData.push_back(start.x);
    m_debugLinesData.push_back(start.y);
    m_debugLinesData.push_back(start.z);
    m_debugLinesData.push_back(color.r);
    m_debugLinesData.push_back(color.g);
    m_debugLinesData.push_back(color.b);
    // end
    m_debugLinesData.push_back(end.x);
    m_debugLinesData.push_back(end.y);
    m_debugLinesData.push_back(end.z);
    m_debugLinesData.push_back(color.r);
    m_debugLinesData.push_back(color.g);
    m_debugLinesData.push_back(color.b);
}

void OpenGLBackend::endDebugLines() {
    if (m_debugLinesData.empty())
        return;

    if (m_debugLinesVao == 0) {
        glGenVertexArrays(1, &m_debugLinesVao);
        glGenBuffers(1, &m_debugLinesVbo);

        glBindVertexArray(m_debugLinesVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_debugLinesVbo);

        const GLsizei stride = sizeof(float) * 6; // vec3 pos + vec3 color
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

        glBindVertexArray(0);
    }

    glBindVertexArray(m_debugLinesVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_debugLinesVbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_debugLinesData.size() * sizeof(float)),
                 m_debugLinesData.data(), GL_DYNAMIC_DRAW);

    glEnable(GL_DEPTH_TEST);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_debugLinesData.size() / 6));

    glBindVertexArray(0);
}

void OpenGLBackend::bindTexture(const std::string &name, unsigned int textureID, int slot) {
    if (!m_currentShader || m_currentShader->programID == 0)
        return;
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLint loc = getUniformLocation(*m_currentShader, name);
    if (loc >= 0)
        glUniform1i(loc, slot);
}