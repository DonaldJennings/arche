#pragma once

#include <memory>
#include <vector>
#include <string>

#include <IPanel.h>

namespace Arche {
    namespace GUI {
        class PanelRegistry {
          public:
            void RegisterPanel(std::shared_ptr<IPanel> panel);

            void DrawPanels();

            std::shared_ptr<IPanel> GetPanel(std::string const &name);

          private:
            std::vector<std::shared_ptr<IPanel>> panels;
        };
    } // namespace GUI
} // namespace Arche