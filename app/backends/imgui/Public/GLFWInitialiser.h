#pragma once

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <cstdio>

namespace Arche {
namespace GUI {

/**
 * @brief RAII wrapper for GLFW initialization and termination.
 */
class GLFWInitialiser {
public:
    GLFWInitialiser() {
        glfwSetErrorCallback([](int error, const char* description) {
            std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
        });
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
    }

    ~GLFWInitialiser() {
        glfwTerminate();
    }

    GLFWInitialiser(const GLFWInitialiser&) = delete;
    GLFWInitialiser& operator=(const GLFWInitialiser&) = delete;
    GLFWInitialiser(GLFWInitialiser&&) = delete;
    GLFWInitialiser& operator=(GLFWInitialiser&&) = delete;
};

} // namespace GUI
} // namespace Arche