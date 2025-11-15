#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "EntityInspector.h"

#include <WorldSystem.h>
#include <imgui.h>

#include <algorithm> // for std::max, std::min
#include <array>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <numeric>
#include <vector>

namespace Arche {
    namespace GUI {

        void EntityInspector::Draw() {
            ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_None);

            ImGui::Text("Entity Inspector Panel");
        
            ImGui::End();
        }

        std::string_view EntityInspector::GetName() const { return name; }

    } // namespace GUI
} // namespace Arche
