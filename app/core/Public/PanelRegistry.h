#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <IPanel.h>

namespace Arche {
    namespace GUI {
        class PanelRegistry {
          public:
            inline void RegisterPanel(std::shared_ptr<IPanel> panel) { panels.insert({panel->GetName(), std::move(panel)}); }

            void DrawPanels() {
                for (const auto [name, panel] : panels) {
                    panel->Draw();
                }
            }

            std::shared_ptr<IPanel> GetPanel(std::string const &name) {
                auto it = panels.find(name);
                return (it == panels.end()) ? nullptr : it->second;
            }

          private:
            std::unordered_map<std::string_view, std::shared_ptr<IPanel>> panels;
        };
    } // namespace GUI
} // namespace Arche