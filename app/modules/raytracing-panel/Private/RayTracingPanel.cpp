#include "RayTracingPanel.h"

#include <imgui.h>
#include <GlobalSettings.h>

namespace Arche {
    namespace GUI {

        void RayTracingPanel::Draw() {
            ImGui::Begin("Ray Tracing");

            if (!m_ctx) {
                ImGui::TextDisabled("No engine context.");
                ImGui::End();
                return;
            }

            auto &pts = m_ctx->globalSettings()
                              .getRenderSettings().pathTrace;

            // ── Enable / disable ──────────────────────────────────────────────
            bool enabled = pts.enabled;
            if (ImGui::Checkbox("Enable Path Tracing", &enabled)) {
                pts.enabled = enabled;
#ifdef ARCHE_BACKEND_VULKAN
                if (m_ptPass) m_ptPass->resetAccumulation();
#endif
            }
            if (!enabled) {
                ImGui::TextDisabled("(Rasterisation active)");
                ImGui::End();
                return;
            }

            ImGui::Separator();

            // ── Accumulation info ─────────────────────────────────────────────
#ifdef ARCHE_BACKEND_VULKAN
            if (m_ptPass) {
                ImGui::Text("Samples accumulated: %u", m_ptPass->getTotalSamples());
            }
#endif

            // ── Quality settings ──────────────────────────────────────────────
            ImGui::SeparatorText("Quality");

            bool changed = false;

            int bounces = pts.maxBounces;
            if (ImGui::SliderInt("Max Bounces", &bounces, 1, 50)) {
                pts.maxBounces = bounces;
                changed = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Maximum ray-bounce depth per path.\n"
                                  "Higher = better indirect lighting, slower.");

            int spf = pts.samplesPerFrame;
            if (ImGui::SliderInt("Samples / Frame", &spf, 1, 16)) {
                pts.samplesPerFrame = spf;
                changed = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("New random samples added each frame.\n"
                                  "Increase for faster convergence at cost of frame time.");

            // ── Camera / DOF ──────────────────────────────────────────────────
            ImGui::SeparatorText("Camera");

            float aperture = pts.aperture;
            if (ImGui::SliderFloat("Aperture", &aperture, 0.0f, 2.0f, "%.3f")) {
                pts.aperture = aperture;
                changed = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Lens aperture for depth-of-field.\n"
                                  "0 = pinhole (no blur).");

            float focus = pts.focusDistance;
            if (ImGui::SliderFloat("Focus Distance", &focus, 0.1f, 100.0f, "%.2f")) {
                pts.focusDistance = focus;
                changed = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("World-space distance to the plane of perfect focus.");

            // ── Reset ─────────────────────────────────────────────────────────
            ImGui::Separator();
            if (ImGui::Button("Reset Accumulation") || changed) {
#ifdef ARCHE_BACKEND_VULKAN
                if (m_ptPass) m_ptPass->resetAccumulation();
#endif
            }

            ImGui::End();
        }

    } // namespace GUI
} // namespace Arche
