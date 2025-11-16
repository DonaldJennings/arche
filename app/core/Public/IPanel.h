#pragma once

#include <string>
#include <string_view>

namespace Arche {
    namespace GUI {
        /**
         * @brief Abstract interface for editor UI panels.
         * 
         * IPanel defines the contract for all UI panels in the editor.
         * Panels are self-contained UI components that can be docked,
         * moved, and hidden in the editor interface.
         * 
         * Examples include log viewers, property inspectors, viewport
         * windows, and asset browsers.
         */
        class IPanel {
          public:
            /**
             * @brief Virtual destructor for proper cleanup.
             */
            virtual ~IPanel() = default;
            
            /**
             * @brief Draw the panel's UI.
             * 
             * Called once per frame to render the panel's ImGui interface.
             * Implementations should use ImGui calls to build the panel UI.
             */
            virtual void Draw() = 0;
            
            /**
             * @brief Get the panel's display name.
             * 
             * Used for window titles and the panels menu.
             * 
             * @return Panel name as string view
             */
            virtual std::string_view GetName() const = 0;
        };

    } // namespace GUI
} // namespace Arche