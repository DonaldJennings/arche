#pragma once

#include <memory>
#include <vector>

#include <IPanel.h>

namespace Arche {
    namespace GUI {
        class PanelRegistry {
          public:
            inline void RegisterPanel(std::shared_ptr<IPanel> panel) { panels.push_back(std::move(panel)); }

            void DrawPanels() {
                for (const auto &panel : panels) {
                    panel->Draw();
                }
            }

            std::shared_ptr<IPanel> GetPanel(std::string const &name) {
                for (auto &panel : panels) {
                    if (panel->GetName() == name) {
                        return panel;
                    }
                }
                return nullptr;
            }

          private:
            std::vector<std::shared_ptr<IPanel>> panels;
        };
    } // namespace GUI
} // namespace Arche