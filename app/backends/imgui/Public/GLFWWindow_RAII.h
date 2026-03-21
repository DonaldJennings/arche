#pragma once

#ifndef ARCHE_BACKEND_VULKAN
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace Arche {
namespace GUI {

/**
 * @brief RAII wrapper for a GLFW window.
 */
class GLFWWindowHandle {
public:
    GLFWWindowHandle(int width, int height, const char* title) {
#ifdef ARCHE_BACKEND_VULKAN
        // Vulkan manages its own context — tell GLFW not to create an OpenGL one
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#else
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
#endif
        window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!window_) {
            throw std::runtime_error("Failed to create GLFW window");
        }

#ifndef ARCHE_BACKEND_VULKAN
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwDestroyWindow(window_);
            throw std::runtime_error("Failed to initialize GLAD");
        }
#endif
    }

    ~GLFWWindowHandle() {
        if (window_) {
            glfwDestroyWindow(window_);
        }
    }

    GLFWWindowHandle(const GLFWWindowHandle&) = delete;
    GLFWWindowHandle& operator=(const GLFWWindowHandle&) = delete;
    GLFWWindowHandle(GLFWWindowHandle&&) = delete;
    GLFWWindowHandle& operator=(GLFWWindowHandle&&) = delete;

    operator GLFWwindow*() { return window_; }
    GLFWwindow* get() const { return window_; }

private:
    GLFWwindow* window_ = nullptr;
};

} // namespace GUI
} // namespace Arche