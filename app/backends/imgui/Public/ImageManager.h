#pragma once

#include <string>
#include <GLFW/glfw3.h>
#include <cstdint>

namespace Arche {
namespace GUI {
    // Load logo image into an OpenGL texture. Returns 0 on failure.
    unsigned int LoadLogoTexture(const std::string& path, int& outWidth, int& outHeight);

    // Return cached logo texture ID (0 if not loaded)
    unsigned int GetLogoTextureID();

    // Set the GLFW window icon from the image file. Returns true on success.
    bool SetWindowIconFromFile(GLFWwindow* window, const std::string& path);
} // namespace GUI
} // namespace Arche
