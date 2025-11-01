#include "ImGuiFontManager.h"

#include <imgui.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")
#else
#include <unistd.h>
#endif

#if defined(IMGUI_IMPL_OPENGL3)
#include <backends/imgui_impl_opengl3.h>
#endif
#if defined(IMGUI_IMPL_GLFW)
#include <backends/imgui_impl_glfw.h>
#endif

#include <string>
#include <filesystem>
#include <cstdio>
#include <vector>

namespace Arche {
namespace GUI {

static ImFont* s_mainFont = nullptr;

static std::filesystem::path GetExecutableDir() {
#ifdef _WIN32
    char buf[MAX_PATH] = {0};
    DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        return std::filesystem::path(buf).parent_path();
    }
    return std::filesystem::current_path();
#else
    char buf[1024];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        return std::filesystem::path(buf).parent_path();
    }
    return std::filesystem::current_path();
#endif
}

static std::filesystem::path FindFontFile(const std::string& name) {
    std::vector<std::filesystem::path> candidates;

    // Current working directory candidates
    candidates.push_back(std::filesystem::path("assets/fonts") / name);
    candidates.push_back(std::filesystem::path("../assets/fonts") / name);
    candidates.push_back(std::filesystem::path("..\\assets\\fonts") / name);
    candidates.push_back(std::filesystem::path(name)); // bare filename next to exe

    // Executable directory candidates (covers build output copy locations)
    auto exeDir = GetExecutableDir();
    candidates.push_back(exeDir / "assets" / "fonts" / name);
    candidates.push_back(exeDir / ".." / "assets" / "fonts" / name);
    candidates.push_back(exeDir / name);

    // Try also the build folder relative to source (useful when running from IDE)
    candidates.push_back(std::filesystem::path("./build/assets/fonts") / name);
    candidates.push_back(std::filesystem::path("../build/assets/fonts") / name);

    for (const auto &p : candidates) {
        if (std::filesystem::exists(p)) {
            return p;
        }
    }

    // Diagnostic: print tried candidates to stderr to help debugging
    std::fprintf(stderr, "[ImGuiFontManager] Font lookup failed. Tried %zu paths:\n", candidates.size());
    for (const auto &p : candidates) {
        std::fprintf(stderr, "  %s\n", p.string().c_str());
    }

    return {};
}

void RebuildImGuiFontsForDpi(float dpiScale) {
    if (!(dpiScale > 0.0f)) dpiScale = 1.0f;

    ImGuiIO& io = ImGui::GetIO();

    // Choose a base pixel size suitable for normal DPI; adjust to taste.
    const float baseFontPx = 10.0f; // lowered from 16px to make font slightly smaller
    const float fontSizePx = baseFontPx * dpiScale;

    // Clear existing fonts and add a new font sized for the DPI.
    io.Fonts->Clear();

    // Font file name (we try several candidate paths)
    const char* fontFileName = "Roboto-Regular.ttf";
    auto fontPath = FindFontFile(fontFileName);

    ImFontConfig cfg;
    cfg.OversampleH = 3;
    cfg.OversampleV = 1;
    cfg.PixelSnapH = true;

    if (!fontPath.empty()) {
        ImFont* f = io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), fontSizePx, &cfg, io.Fonts->GetGlyphRangesDefault());
        if (!f) {
            // If AddFontFromFileTTF failed despite file existing, fallback to default
            std::fprintf(stderr, "[ImGuiFontManager] Failed to build font atlas from '%s', falling back to default font.\n", fontPath.string().c_str());
            io.Fonts->AddFontDefault();
        } else {
            s_mainFont = f;
        }
    } else {
        std::fprintf(stderr, "[ImGuiFontManager] Font file not found. Looked for '%s' under common locations. Falling back to default font.\n", fontFileName);
        io.Fonts->AddFontDefault();
    }

    // Build the atlas now
    io.Fonts->Build();

    // If we loaded a font, make it the default so all ImGui widgets use Roboto
    if (s_mainFont) {
        io.FontDefault = s_mainFont;
    } else {
        io.FontDefault = nullptr;
    }

    // Some backends maintain GPU-side font textures; recreate them now.
#if defined(IMGUI_IMPL_OPENGL3)
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    ImGui_ImplOpenGL3_CreateDeviceObjects();
#endif

    // We do NOT rely on FontGlobalScale for DPI; leave it at 1.0 to avoid blurring.
    io.FontGlobalScale = 1.0f;
}

float GetPlatformDpiScale() {
#ifdef _WIN32
    // Try to get DPI for the main monitor / primary window. Returns scale relative to 96 DPI.
    float scale = 1.0f;

    // Prefer GetDpiForWindow if available (Win10+)
    HWND hwnd = GetActiveWindow();
    if (hwnd) {
        UINT dpi = ::GetDpiForWindow(hwnd); // Windows 10
        if (dpi > 0) {
            scale = static_cast<float>(dpi) / 96.0f;
            return scale;
        }
    }

    // Fallback to system DPI
    HDC screen = GetDC(NULL);
    if (screen) {
        int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
        ReleaseDC(NULL, screen);
        if (dpiX > 0) scale = static_cast<float>(dpiX) / 96.0f;
    }
    return scale;
#else
    // On other platforms you can query the display scaling via your windowing system.
    return 1.0f;
#endif
}

} // namespace GUI
} // namespace Arche
