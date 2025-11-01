#pragma once

#include "imgui.h"

namespace Arche
{
    namespace GUI
    {
        inline void setDarkTheme(bool bStyleDark_, float alpha_)
        {
            ImGuiStyle &style = ImGui::GetStyle();

            // Layout
            style.WindowPadding = ImVec2(10.0f, 10.0f);
            style.FramePadding = ImVec2(8.0f, 6.0f);
            style.ItemSpacing = ImVec2(8.0f, 6.0f);
            style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
            style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
            style.IndentSpacing = 12.0f;
            style.ScrollbarSize = 12.0f;
            style.GrabMinSize = 10.0f;

            // Rounding
            style.WindowRounding = 6.0f;
            style.ChildRounding = 6.0f;
            style.FrameRounding = 6.0f;
            style.ScrollbarRounding = 6.0f;
            style.GrabRounding = 6.0f;

            // Alpha
            style.Alpha = alpha_;

            // Dark minimalist palette (muted grays + subtle accent)
            const ImVec4 bg           = ImVec4(0.12f, 0.13f, 0.15f, 1.00f); // main background
            const ImVec4 bg_alt       = ImVec4(0.16f, 0.17f, 0.19f, 1.00f); // panels/child
            const ImVec4 panel        = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
            const ImVec4 border       = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
            const ImVec4 text         = ImVec4(0.92f, 0.93f, 0.94f, 1.00f);
            const ImVec4 text_dim     = ImVec4(0.60f, 0.62f, 0.64f, 1.00f);
            const ImVec4 accent       = ImVec4(0.18f, 0.65f, 0.58f, 1.00f); // teal/cyan accent
            const ImVec4 accent_soft  = ImVec4(0.18f, 0.65f, 0.58f, 0.18f);
            const ImVec4 white_a12    = ImVec4(1.0f, 1.0f, 1.0f, 0.12f);

            ImVec4* colors = style.Colors;
            // Windows and panels
            colors[ImGuiCol_Text]                   = text;
            colors[ImGuiCol_TextDisabled]           = text_dim;
            colors[ImGuiCol_WindowBg]               = bg;
            colors[ImGuiCol_ChildBg]                = bg_alt;
            colors[ImGuiCol_PopupBg]                = bg_alt;
            colors[ImGuiCol_Border]                 = border;
            colors[ImGuiCol_BorderShadow]           = ImVec4(0,0,0,0);

            // Frames, inputs
            colors[ImGuiCol_FrameBg]                = ImVec4(0.16f,0.17f,0.18f,1.00f);
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.20f,0.22f,0.24f,1.00f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.22f,0.24f,0.26f,1.00f);

            // Title and menu
            colors[ImGuiCol_TitleBg]                = ImVec4(0.10f,0.11f,0.12f,1.00f);
            colors[ImGuiCol_TitleBgActive]          = ImVec4(0.12f,0.13f,0.15f,1.00f);
            colors[ImGuiCol_MenuBarBg]              = bg_alt;

            // Scrollbars
            colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.0f,0.0f,0.0f,0.0f);
            colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.22f,0.24f,0.26f,1.00f);
            colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.26f,0.28f,0.30f,1.00f);
            colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.28f,0.30f,0.32f,1.00f);

            // Buttons
            colors[ImGuiCol_Button]                 = ImVec4(accent.x, accent.y, accent.z, 0.12f);
            colors[ImGuiCol_ButtonHovered]          = ImVec4(accent.x, accent.y, accent.z, 0.90f);
            colors[ImGuiCol_ButtonActive]           = ImVec4(accent.x * 0.9f, accent.y * 0.9f, accent.z * 0.9f, 1.00f);

            // Headers (tree nodes, etc.)
            colors[ImGuiCol_Header]                 = ImVec4(0.18f,0.19f,0.20f,1.00f);
            colors[ImGuiCol_HeaderHovered]          = ImVec4(accent.x, accent.y, accent.z, 0.14f);
            colors[ImGuiCol_HeaderActive]           = ImVec4(accent.x, accent.y, accent.z, 0.28f);

            // Separators
            colors[ImGuiCol_Separator]              = ImVec4(0.18f,0.19f,0.20f,1.00f);
            colors[ImGuiCol_SeparatorHovered]       = ImVec4(accent.x, accent.y, accent.z, 0.18f);
            colors[ImGuiCol_SeparatorActive]        = ImVec4(accent.x, accent.y, accent.z, 0.30f);

            // Resize grip
            colors[ImGuiCol_ResizeGrip]             = white_a12;
            colors[ImGuiCol_ResizeGripHovered]      = ImVec4(1.0f,1.0f,1.0f,0.20f);
            colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.0f,1.0f,1.0f,0.30f);

            // Tabs
            colors[ImGuiCol_Tab]                    = ImVec4(0.14f,0.15f,0.17f,1.00f);
            colors[ImGuiCol_TabHovered]             = ImVec4(accent.x, accent.y, accent.z, 0.18f);
            colors[ImGuiCol_TabActive]              = ImVec4(accent.x, accent.y, accent.z, 0.28f);
            colors[ImGuiCol_TabUnfocused]           = ImVec4(0.12f,0.13f,0.15f,1.00f);
            colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f,0.15f,0.17f,1.00f);

            // Plot and Text selection
            colors[ImGuiCol_PlotLines]              = ImVec4(0.55f,0.55f,0.55f,1.00f);
            colors[ImGuiCol_PlotLinesHovered]       = ImVec4(accent.x, accent.y, accent.z, 1.00f);
            colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f,0.70f,0.00f,1.00f);
            colors[ImGuiCol_TextSelectedBg]         = ImVec4(accent.x, accent.y, accent.z, 0.20f);

            // Disabled/others
            colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.0f,0.0f,0.0f,0.45f);

            // Optionally apply alpha to transparent colors
            if (alpha_ < 1.0f) {
                for (int i = 0; i <= ImGuiCol_COUNT; ++i) {
                    colors[i].w *= alpha_;
                }
            }
        }
    }
}