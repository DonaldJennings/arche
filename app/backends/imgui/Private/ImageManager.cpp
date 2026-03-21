#include "ImageManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <filesystem>

#ifndef ARCHE_BACKEND_VULKAN
#include <glad/glad.h>
#endif

namespace Arche {
namespace GUI {

static unsigned int s_logoTex = 0;

unsigned int LoadLogoTexture(const std::string& path, int& outWidth, int& outHeight) {
    if (s_logoTex != 0) {
        outWidth = 0; outHeight = 0;
        return s_logoTex;
    }

    std::filesystem::path p(path);
    if (!std::filesystem::exists(p)) {
        std::cerr << "Logo file not found: " << path << std::endl;
        return 0;
    }

    int channels = 0;
    unsigned char* data = stbi_load(path.c_str(), &outWidth, &outHeight, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image: " << path << std::endl;
        return 0;
    }

#ifndef ARCHE_BACKEND_VULKAN
    glGenTextures(1, &s_logoTex);
    glBindTexture(GL_TEXTURE_2D, s_logoTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outWidth, outHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
#else
    (void)data; // Vulkan: logo texture upload not yet implemented
    s_logoTex = 1; // non-zero sentinel so we don't retry
#endif

    stbi_image_free(data);
    return s_logoTex;
}

unsigned int GetLogoTextureID() { return s_logoTex; }

bool SetWindowIconFromFile(GLFWwindow* window, const std::string& path) {
    if (!window) return false;

    int w, h, channels;
    unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!pixels) {
        std::cerr << "Failed to load icon: " << path << std::endl;
        return false;
    }

    GLFWimage img;
    img.width = w;
    img.height = h;
    img.pixels = pixels;

    glfwSetWindowIcon(window, 1, &img);

    stbi_image_free(pixels);
    return true;
}

} // namespace GUI
} // namespace Arche
