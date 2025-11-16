#pragma once

#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include <IPanel.h>

namespace Arche {
    namespace GUI {
        /**
         * @brief Registry for managing editor UI panels.
         * 
         * PanelRegistry maintains a collection of all active panels in the editor.
         * It provides methods to register new panels, draw all panels each frame,
         * and look up panels by name.
         * 
         * The registry ensures all panels are drawn in a consistent order and
         * provides centralized panel management for the editor.
         */
        class PanelRegistry {
          public:
            /**
             * @brief Register a new panel with the editor.
             * 
             * Adds a panel to the active panels list. The panel will be drawn
             * every frame and can be looked up by name.
             * 
             * @param panel Shared pointer to the panel to register
             */
            inline void RegisterPanel(std::shared_ptr<IPanel> panel) { panels.push_back(std::move(panel)); }

            /**
             * @brief Draw all registered panels.
             * 
             * Calls Draw() on each registered panel. Should be called once per
             * frame during the UI rendering phase.
             */
            void DrawPanels() {
                for (const auto &panel : panels) {
                    if (panel) panel->Draw();
                }
            }

            /**
             * @brief Find a panel by name.
             * 
             * Searches the registered panels for one with the given name.
             * 
             * @param name Panel name to search for
             * @return Shared pointer to the panel, or nullptr if not found
             */
            std::shared_ptr<IPanel> GetPanel(std::string const &name) {
                auto it = std::find_if(panels.begin(), panels.end(), [&](const std::shared_ptr<IPanel> &p) {
                    return p && std::string(p->GetName()) == name;
                });
                return (it == panels.end()) ? nullptr : *it;
            }

          private:
            std::vector<std::shared_ptr<IPanel>> panels;  ///< Active panels
        };
    } // namespace GUI
} // namespace Arche