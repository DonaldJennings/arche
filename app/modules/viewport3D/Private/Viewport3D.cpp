#pragma once
#include "Viewport3D.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <UIContext.h>
#include <World.h>
#include <WorldSystem.h>
#include <CameraController.h>
#include <Camera.h>

#include <memory>
#include <vector>
#include <optional>
#include <algorithm>
#include <sstream>

namespace {
    struct EditState {
        double posX = 0, posY = 0, mass = 1, scale = 1;
        std::uint64_t editingId = 0;
    };

    std::optional<std::uint64_t> selectedBodyId;
    std::optional<EditState> editState;
    ImVec2 cameraOffset{0.0f, 0.0f};

    inline ImVec2 ImVec2Subtract(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x - b.x, a.y - b.y); }
} // anonymous

namespace Arche {
namespace GUI {

Viewport3DPanel::Viewport3DPanel(std::shared_ptr<Arche::GUI::UIContext> panelContext)
    : name{"3DViewport"}, context(std::move(panelContext)) {
    // nothing else for now
}

void Viewport3DPanel::Draw() {
    ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_None);

        auto camera = context->renderer()->getAttachedCamera();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 200.0f);
        ImGui::Text("Camera Pos: (%.1f, %.1f, %.1f)", camera->GetPosition().x(), camera->GetPosition().y(),
                    camera->GetPosition().z());

    // Determine UI scale for high-DPI displays (use framebuffer scale)
    ImGuiIO &io = ImGui::GetIO();
    float dpiScale = io.DisplayFramebufferScale.x;
    if (!(dpiScale > 0.0f)) dpiScale = 1.0f;
    float uiScale = dpiScale;

    // Access world system
    auto worldSystem = context->worldSystem();
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

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    // Background (under renderer image)
    ImU32 bg = IM_COL32(30, 30, 40, 255);
    drawList->AddRectFilled(canvasP0, canvasP1, bg);
    drawList->AddRect(canvasP0, canvasP1, IM_COL32(80,80,90,255), 0.0f, 0, 1.0f);

    // Draw renderer texture (if any) clipped to canvas
    if (context && context->renderer()) {
        unsigned int tex = context->renderer()->getRenderTexture();
        // resize handling (recreate FBO if canvas size changed)
        int rw = static_cast<int>(canvasSize.x);
        int rh = static_cast<int>(canvasSize.y);
        if (rw > 0 && rh > 0 && (context->renderer()->getWidth() != rw || context->renderer()->getHeight() != rh)) {
            context->renderer()->setViewportSize(rw, rh);
            context->renderer()->recreateFrameBuffer();

            // Immediately render current world into new framebuffer so UI samples valid pixels
            if (world) {
                context->renderer()->render(world->view().bodies);
            }
        }

        if (tex != 0) {
            drawList->PushClipRect(canvasP0, canvasP1, true);
            ImVec2 uv0(0.0f, 1.0f);
            ImVec2 uv1(1.0f, 0.0f);
            drawList->AddImage((void*)(intptr_t)tex, canvasP0, canvasP1, uv0, uv1);
            drawList->PopClipRect();
        }
    }

    // --- Interaction: selection, add, edit, draw markers ---
    ImVec2 mousePos{ImGui::GetIO().MousePos};
    bool mouseClicked{ImGui::IsMouseClicked(0)};

    auto view = world->view();

    // Draw bodies overlays and handle selection (clip to canvas)
    drawList->PushClipRect(canvasP0, canvasP1, true);

    for (const auto &body : view.bodies) {
        // Map world/object pos to canvas screen pos (same simple mapping as 2D POC)
        ImVec2 pos = ImVec2(
            canvasP0.x + static_cast<float>(body.transform.getPosition().x()) - cameraOffset.x,
            canvasP0.y + static_cast<float>(body.transform.getPosition().y()) - cameraOffset.y
        );

        float baseRadius = 25.0f * uiScale;
        float scale = static_cast<float>(body.transform.getScale().x());
        float radius = baseRadius * scale;

        // selection hit test
        if (mouseClicked && ImGui::IsWindowHovered() && ImLengthSqr(ImVec2Subtract(mousePos, pos)) < radius * radius) {
            std::ostringstream oss;
            oss << "Clicked on particle id " << body.id << " at position (" << body.transform.getPosition().x() << ", " << body.transform.getPosition().y() << ")";
            ARCHE_LOG_INFO(context->logger(), oss.str());
            selectedBodyId = body.id;
            auto p = body.transform.getPosition();
            editState = EditState{p.x(), p.y(), body.mass, body.transform.getScale().x(), body.id};
        }
    }

    drawList->PopClipRect();

    // Context menu: Add Particle
    if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("ViewportContextMenu");
    }

    if (ImGui::BeginPopup("ViewportContextMenu")) {
        if (ImGui::MenuItem("Add Particle Here")) {
            ImVec2 relPos = ImVec2(mousePos.x - canvasP0.x + cameraOffset.x, mousePos.y - canvasP0.y + cameraOffset.y);
            Arche::Math::SpatialTransform transform;
            transform.setPosition(Arche::Math::Vector3D(relPos.x, relPos.y, 0.0f));
            world->createParticle(transform, 1.0f);
        }
        ImGui::EndPopup();
    }

    // If selection exists, open edit popup
    auto selectedIt = selectedBodyId ? std::find_if(view.bodies.begin(), view.bodies.end(), [&](const auto &b) { return b.id == *selectedBodyId; }) : view.bodies.end();

    if (selectedBodyId && selectedIt != view.bodies.end()) {
        ImGui::OpenPopup("EditObjectPopup");
    }

    if (ImGui::BeginPopupModal("EditObjectPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (selectedIt != view.bodies.end() && editState && editState->editingId == selectedIt->id) {
            ImGui::InputDouble("Position X: ", &editState->posX);
            ImGui::InputDouble("Position Y: ", &editState->posY);
            ImGui::InputDouble("Mass: ", &editState->mass);
            ImGui::InputDouble("Scale: ", &editState->scale);

            if (ImGui::Button("Apply")) {
                world->setObjectPosition(selectedIt->id, Arche::Math::Vector3D(editState->posX, editState->posY, 0.0f));
                world->setObjectMass(selectedIt->id, editState->mass);
                world->setObjectScale(selectedIt->id, editState->scale);
                ImGui::CloseCurrentPopup();
                selectedBodyId.reset();
                editState.reset();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
                selectedBodyId.reset();
                editState.reset();
            }
        } else {
            ImGui::CloseCurrentPopup();
            selectedBodyId.reset();
            editState.reset();
        }
        ImGui::EndPopup();
    }

    // Camera/keyboard panning (same as 2D)
    ImGuiIO& io2 = ImGui::GetIO();
    bool viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    ImGui::End();
}

std::string_view Viewport3DPanel::GetName() const { return name; }

void Viewport3DPanel::Reset() {
    ARCHE_LOG_WARNING(context->logger(), "Resetting 3D viewport (no-op)");
}

} // namespace GUI
} // namespace Arche