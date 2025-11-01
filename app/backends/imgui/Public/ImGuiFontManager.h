#pragma once

#include <imgui.h>

namespace Arche {
namespace GUI {
    // Rebuild the ImGui font atlas to use fonts sized for the given DPI scale (1.0 = 96 DPI).
    // Must be called after ImGui context is created and backend is initialized.
    void RebuildImGuiFontsForDpi(float dpiScale);

    // Helper to query DPI scale on Windows (returns 1.0 on other platforms).
    float GetPlatformDpiScale();
} // namespace GUI
} // namespace Arche
