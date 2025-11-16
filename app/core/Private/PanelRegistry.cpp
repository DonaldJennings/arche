#include "PanelRegistry.h"

#include <algorithm>

namespace Arche {
    namespace GUI {

        void PanelRegistry::RegisterPanel(std::shared_ptr<IPanel> panel) {
            panels.push_back(std::move(panel));
        }

        void PanelRegistry::DrawPanels() {
            for (const auto &panel : panels) {
                if (panel) panel->Draw();
            }
        }

        std::shared_ptr<IPanel> PanelRegistry::GetPanel(std::string const &name) {
            auto it = std::find_if(panels.begin(), panels.end(), [&](const std::shared_ptr<IPanel> &p) {
                return p && std::string(p->GetName()) == name;
            });
            return (it == panels.end()) ? nullptr : *it;
        }

    } // namespace GUI
} // namespace Arche
