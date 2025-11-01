#include "Viewport3D.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <UIContext.h>
#include <World.h>
#include <WorldSystem.h>
#include <CameraController.h>
#include <Camera.h>

#include <memory>

namespace Arche {
namespace GUI {

Viewport3DPanel::Viewport3DPanel(std::shared_ptr<Arche::GUI::UIContext> panelContext)
    : name{"3DViewport"}, context(std::move(panelContext)) {
    // nothing else for now
}

void Viewport3DPanel::Draw() {
    ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_None);

    // Access world system
    auto worldSystem = context->worldSystem;
    if (!worldSystem) {
        ImGui::Text("World system not available.");
        ImGui::End();
        return;
    }

    auto world = worldSystem->getWorld();
    if (!world) {
        ImGui::Text("World not available.");
        ImGui::End();
        return;
    }

    // Reserve canvas
    ImVec2 canvasP0 = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
    if (canvasSize.y < 50.0f) canvasSize.y = 50.0f;
    ImVec2 canvasP1 = ImVec2(canvasP0.x + canvasSize.x, canvasP0.y + canvasSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Background
    ImU32 bg = IM_COL32(30, 30, 40, 255);
    drawList->AddRectFilled(canvasP0, canvasP1, bg);
    drawList->AddRect(canvasP0, canvasP1, IM_COL32(80,80,90,255), 0.0f, 0, 1.0f);

    // Camera handling: singleton controller attached to WorldSystem's camera
    static CameraController controller;

    // Show camera world position at top-left of canvas
    auto camPos = Arche::Math::Vector3D{0.0, 0.0, 0.0};
    char buf[128];
    snprintf(buf, sizeof(buf), "Cam: X=%.2f Y=%.2f Z=%.2f", camPos.x(), camPos.y(), camPos.z());
    
    // Print pitch / yaw angles as degrees
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " Pitch=%.1f Yaw=%.1f", controller.getPitchDegrees(), controller.getYawDegrees());
    drawList->AddText(ImVec2(canvasP0.x + 6, canvasP0.y + 6), IM_COL32(220,220,220,255), buf);

    // Keyboard input controls (when viewport is focused)
    ImGuiIO& io = ImGui::GetIO();
    float dt = io.DeltaTime > 0.0f ? io.DeltaTime : (1.0f / 60.0f);

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        // WASD for pan in X/Y (screen plane)
        float panX = 0.0f, panY = 0.0f;
        if (ImGui::IsKeyDown(ImGuiKey_W)) panY -= 1.0f;
        if (ImGui::IsKeyDown(ImGuiKey_S)) panY += 1.0f;
        if (ImGui::IsKeyDown(ImGuiKey_A)) panX -= 1.0f;
        if (ImGui::IsKeyDown(ImGuiKey_D)) panX += 1.0f;

        // Shift / Ctrl for vertical movement
        float vert = 0.0f;
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) vert += 1.0f; // up
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)  || ImGui::IsKeyDown(ImGuiKey_RightCtrl))  vert -= 1.0f; // down

        // Arrow keys for rotation (yaw / pitch)
        float rotYaw = 0.0f, rotPitch = 0.0f;
        if (ImGui::IsKeyDown(ImGuiKey_LeftArrow))  rotYaw -= 1.0f;
        if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) rotYaw += 1.0f;
        if (ImGui::IsKeyDown(ImGuiKey_UpArrow))    rotPitch -= 1.0f;
        if (ImGui::IsKeyDown(ImGuiKey_DownArrow))  rotPitch += 1.0f;

        // Movement scaling
        const float keyboardPanSpeed = 5.0f;    // units per second (multiplied by controller.pan semantics)
        const float keyboardVertSpeed = 3.0f;   // vertical units per second
        const float keyboardRotSpeed = 3.0f;    // arbitrary unit per second processed by controller.rotate

        if (panX != 0.0f || panY != 0.0f) {
            controller.pan(panX * keyboardPanSpeed * dt, panY * keyboardPanSpeed * dt);
        }

        if (vert != 0.0f) {
            // use pan's Y component to move target up/down (second parameter controls some vertical component)
            controller.pan(0.0f, -vert * keyboardVertSpeed * dt);
        }

        if (rotYaw != 0.0f || rotPitch != 0.0f) {
            // controller.rotate multiplies incoming values by rotateSpeed internally; pass a scaled value
            controller.rotate(rotYaw * keyboardRotSpeed * dt, rotPitch * keyboardRotSpeed * dt);
        }

        // Apply updates to camera if any input happened
        if (panX != 0.0f || panY != 0.0f || vert != 0.0f || rotYaw != 0.0f || rotPitch != 0.0f) {
            controller.updateCamera();
        }
    }

    // Simple placeholder: display text and camera controls
    ImVec2 textPos = ImVec2(canvasP0.x + 10.0f, canvasP0.y + 30.0f);
    drawList->AddText(textPos, IM_COL32(220,220,220,255), "3D viewport - basic placeholder");

    ImGui::End();
}

std::string_view Viewport3DPanel::GetName() const { return name; }

void Viewport3DPanel::Reset() {
    ARCHE_LOG_WARNING(context->logger, "Resetting 3D viewport (no-op)");
}

} // namespace GUI
} // namespace Arche