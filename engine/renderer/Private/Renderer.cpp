#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Renderer.h"
#include <iostream>
#include <cstring>

// minimal point sprite shader sources - updated to use view/projection matrices
static const char *ptsVs = R"glsl(
#version 460 core
layout(location = 0) in vec3 inPosition;
uniform mat4 uView;
uniform mat4 uProj;
uniform float uPointSize;
void main() {
    // transform from world-space to clip-space
    gl_Position = uProj * uView * vec4(inPosition, 1.0);
    gl_PointSize = uPointSize;
}
)glsl";

static const char *ptsFs = R"glsl(
#version 460 core
out vec4 fragColor;
uniform vec3 uPointColor;
void main() {
    float r = dot(gl_PointCoord - vec2(0.5), gl_PointCoord - vec2(0.5));
    if (r > 0.25) {
        discard; // make points circular
    }
    fragColor = vec4(uPointColor, 1.0);
}
)glsl";


static GLuint compileShader(GLenum type, const char* source, std::shared_ptr<Arche::Core::LoggingService> logger) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        ARCHE_LOG_ERROR(logger, std::string("Shader compilation failed: ") + infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader, std::shared_ptr<Arche::Core::LoggingService> logger) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        ARCHE_LOG_ERROR(logger, std::string("Program linking failed: ") + infoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

// Static minimal GL objects for POC (created on first render)
static GLuint s_ptsProgram = 0;
static GLuint s_ptsVAO = 0;
static GLuint s_ptsVBO = 0; 


bool Arche::Render::Renderer::initialise(GLFWwindow* mainWindow) {
    // Save current context so we can restore it
    GLFWwindow *previousContext = glfwGetCurrentContext();

    // If caller supplied the application's main window, use that context to create GL resources.
    if (mainWindow) {
        externalContextWindow = mainWindow;
        glfwMakeContextCurrent(externalContextWindow);

        // Ensure GL funcs are loaded (gladLoadGLLoader ok to call again)
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            ARCHE_LOG_ERROR(mLogger, "Failed to initialize GLAD in main context");
            if (previousContext) glfwMakeContextCurrent(previousContext);
            return false;
        }

        if (!initialiseFrameBuffer()) {
            ARCHE_LOG_ERROR(mLogger, "Failed to initialize framebuffer in main context");
            if (previousContext) glfwMakeContextCurrent(previousContext);
            return false;
        }
    } else {
        // Fallback: create a hidden window and create resources in its context
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window) {
            ARCHE_LOG_ERROR(mLogger, "Failed to create hidden GLFW window for renderer");
            if (previousContext) glfwMakeContextCurrent(previousContext);
            return false;
        }

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            ARCHE_LOG_ERROR(mLogger, "Failed to initialize GLAD in renderer hidden context");
            if (previousContext) glfwMakeContextCurrent(previousContext);
            return false;
        }

        if (!initialiseFrameBuffer()) {
            ARCHE_LOG_ERROR(mLogger, "Failed to initialize framebuffer in hidden renderer context");
            if (previousContext) glfwMakeContextCurrent(previousContext);
            return false;
        }
    }

    initialised = true;
    ARCHE_LOG_INFO(mLogger, "Renderer initialized successfully");
    ARCHE_LOG_INFO(mLogger, "OpenGL version: " + std::string((const char *)glGetString(GL_VERSION)));

    // Restore previous context
    if (previousContext) glfwMakeContextCurrent(previousContext);
    return true;
}

void Arche::Render::Renderer::render(std::vector<Arche::Scene::BodyView> const &entities) {
    if (!initialised) {
        ARCHE_LOG_ERROR(mLogger, "Renderer not initialised before attempting to render!");
        return;
    }

    if (!mainViewportCamera) {
        ARCHE_LOG_ERROR(mLogger, "Renderer::render called without a camera set. Call setCamera(...) first.");
        return;
    }

    // Choose which context to make current: if we created a hidden window, use it;
    // otherwise the app's main context (externalContextWindow) must already be current in main loop.
    GLFWwindow *previousContext = glfwGetCurrentContext();
    if (window) {
        glfwMakeContextCurrent(window);
    } else if (externalContextWindow) {
        // We assume the application's main loop has the main context current before calling renderer->render.
        // But to be safe, make it current here.
        glfwMakeContextCurrent(externalContextWindow);
    }

    // Render to our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- Minimal point-sprite rendering POC ---
    // Create program + buffers lazily on first render
    if (s_ptsProgram == 0) {
        GLuint vs = compileShader(GL_VERTEX_SHADER, ptsVs, mLogger);
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, ptsFs, mLogger);
        if (vs && fs) {
            s_ptsProgram = linkProgram(vs, fs, mLogger);
        }
        if (vs)
            glDeleteShader(vs);
        if (fs)
            glDeleteShader(fs);

        // create VAO/VBO
        glGenVertexArrays(1, &s_ptsVAO);
        glGenBuffers(1, &s_ptsVBO);
        glBindVertexArray(s_ptsVAO);
        glBindBuffer(GL_ARRAY_BUFFER, s_ptsVBO);
        // attribute 0: vec3 position (world-space)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glBindVertexArray(0);
    }

    if (s_ptsProgram == 0) {
        ARCHE_LOG_ERROR(mLogger, "Cannot render particles: point sprite shader program not available");
        // Restore previous context for the rest of the application (UI, main window, etc.)
        if (previousContext) {
            glfwMakeContextCurrent(previousContext);
        }
        return;
    }

    glUseProgram(s_ptsProgram);

    // Convert camera matrices (double) to float arrays for GL
    auto viewD = mainViewportCamera->GetViewMatrix();
    auto projD = mainViewportCamera->GetProjectionMatrix();
    float viewF[16];
    float projF[16];
    for (int i = 0; i < 16; ++i) {
        viewF[i] = static_cast<float>(viewD[i]);
        projF[i] = static_cast<float>(projD[i]);
    }

    // upload matrix uniforms once per frame
    GLint locView = glGetUniformLocation(s_ptsProgram, "uView");
    GLint locProj = glGetUniformLocation(s_ptsProgram, "uProj");
    if (locView >= 0) glUniformMatrix4fv(locView, 1, GL_FALSE, viewF);
    if (locProj >= 0) glUniformMatrix4fv(locProj, 1, GL_FALSE, projF);

    // uniform locations that are constant across particles
    GLint locPointSize = glGetUniformLocation(s_ptsProgram, "uPointSize");
    GLint locColor = glGetUniformLocation(s_ptsProgram, "uPointColor");
    if (locColor >= 0) glUniform3f(locColor, 0.9f, 0.6f, 0.2f);

    // Loop over entities and draw each particle at its position (world-space).
    for (const auto &ent : entities) {
        // Extract particle position (world coordinates).
        float px = static_cast<float>(ent.transform.getPosition().x());
        float py = static_cast<float>(ent.transform.getPosition().y());
        float pz = static_cast<float>(ent.transform.getPosition().z());
        float pos[3] = { px, py, pz };

        // upload single position
        glBindBuffer(GL_ARRAY_BUFFER, s_ptsVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_DYNAMIC_DRAW);

        // per-particle point size based on ent.scale
        float radiusWorld = static_cast<float>(ent.transform.getScale().x());
        // A simple world-space to pixel-sized heuristic: use radiusWorld * k, or just use radiusWorld
        float pointSize = std::max(1.0f, radiusWorld * 2.0f);
        if (locPointSize >= 0) glUniform1f(locPointSize, pointSize);

        // draw single point
        glBindVertexArray(s_ptsVAO);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);

        // restore minimal state for next particle (kept simple)
        glDisable(GL_PROGRAM_POINT_SIZE);
        glDisable(GL_BLEND);
    }

    glUseProgram(0);

    // restore some state (conservative)
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Restore previous context
    if (previousContext) {
        glfwMakeContextCurrent(previousContext);
    }
}

bool Arche::Render::Renderer::initialiseFrameBuffer() {
    // Clean up old attachments
    if (renderTexture) {
        glDeleteTextures(1, &renderTexture);
        renderTexture = 0;
    }
    if (renderBuffer) {
        glDeleteRenderbuffers(1, &renderBuffer);
        renderBuffer = 0;
    }
    if (frameBuffer) {
        glDeleteFramebuffers(1, &frameBuffer);
        frameBuffer = 0;
    }

    // Ensure proper alignment for GL uploads (defensive)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Create texture for color attachment
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    // Use RGBA for generality; change if you need only RGB
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create frame buffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

    // Create renderbuffer for depth / stencil attachment
    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        ARCHE_LOG_ERROR(mLogger, std::string("Framebuffer is not complete! Status: 0x") +
            [](GLenum s) {
                char buf[16];
                std::snprintf(buf, sizeof(buf), "%X", static_cast<unsigned>(s));
                return std::string(buf);
            }(status));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    // Important: clear the newly-created framebuffer to initialize texture contents.
    // This prevents sampling uninitialized memory (green/pink artifacts) when UI samples the texture.
    glClearColor(0.10f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void Arche::Render::Renderer::shutdown() noexcept {
    // Save current context and make our context current so GL deletions happen in the correct context.
    GLFWwindow *previousContext = glfwGetCurrentContext();
    GLFWwindow *ctx = window ? window : externalContextWindow;
    if (ctx) {
        glfwMakeContextCurrent(ctx);
    }

    if (renderTexture) {
        glDeleteTextures(1, &renderTexture);
        renderTexture = 0;
    }
    if (renderBuffer) {
        glDeleteRenderbuffers(1, &renderBuffer);
        renderBuffer = 0;
    }
    if (frameBuffer) {
        glDeleteFramebuffers(1, &frameBuffer);
        frameBuffer = 0;
    }

    // Only destroy the hidden window if we created one.
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    // Do not destroy externalContextWindow (we don't own it)

    // Restore previous context
    if (previousContext) {
        glfwMakeContextCurrent(previousContext);
    }

    initialised = false;
    ARCHE_LOG_INFO(mLogger, "Renderer shutdown completed");
}

bool Arche::Render::Renderer::recreateFrameBuffer() {
    if (!initialised)
        return false;

    GLFWwindow *previousContext = glfwGetCurrentContext();
    GLFWwindow *ctx = window ? window : externalContextWindow;
    if (ctx) glfwMakeContextCurrent(ctx);

    // Delete previous attachments
    if (frameBuffer) {
        glDeleteFramebuffers(1, &frameBuffer);
        frameBuffer = 0;
    }
    if (renderTexture) {
        glDeleteTextures(1, &renderTexture);
        renderTexture = 0;
    }
    if (renderBuffer) {
        glDeleteRenderbuffers(1, &renderBuffer);
        renderBuffer = 0;
    }

    bool ok = initialiseFrameBuffer();

    if (previousContext)
        glfwMakeContextCurrent(previousContext);
    return ok;
}
