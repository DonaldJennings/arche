#pragma once

#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include <IPanel.h>

namespace Arche {
    namespace GUI {
        class PanelRegistry {
          public:
            inline void RegisterPanel(std::shared_ptr<IPanel> panel) { panels.push_back(std::move(panel)); }

            void DrawPanels() {
                for (const auto &panel : panels) {
                    if (panel) panel->Draw();
                }
            }

            std::shared_ptr<IPanel> GetPanel(std::string const &name) {
                auto it = std::find_if(panels.begin(), panels.end(), [&](const std::shared_ptr<IPanel> &p) {
                    return p && std::string(p->GetName()) == name;
                });
                return (it == panels.end()) ? nullptr : *it;
            }

          private:
            std::vector<std::shared_ptr<IPanel>> panels;
        };
    } // namespace GUI
} // namespace Arche