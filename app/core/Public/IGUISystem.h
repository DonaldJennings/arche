#ifndef I_GUISYSTEM_H
#define I_GUISYSTEM_H

#include <functional>
#include <memory>

#include "IPanel.h"
#include "EditorSession.h"

namespace Arche {
    namespace GUI {

        /**
         * @brief Abstract interface for the GUI rendering system.
         * 
         * IGUISystem defines the lifecycle and rendering interface for
         * the editor's GUI layer. It manages ImGui initialization,
         * frame rendering, and integration with the windowing system.
         * 
         * Concrete implementations (like ImGuiBackend) handle the actual
         * ImGui setup and rendering for specific platforms (GLFW, SDL, etc.).
         */
        class IGUISystem
        {
          public:
              /**
               * @brief Virtual destructor for proper cleanup.
               */
              virtual ~IGUISystem() = default;
              
              /**
               * @brief Initialize the GUI system.
               * 
               * Sets up ImGui, loads fonts, configures the style, and
               * prepares for rendering. Must be called before rendering.
               */
              virtual void Startup() = 0;
              
              /**
               * @brief Shutdown the GUI system.
               * 
               * Cleans up ImGui resources and releases the GUI context.
               * Should be called before destroying the GUI system.
               */
              virtual void Shutdown() = 0;
              
              /**
               * @brief Begin a new GUI frame.
               * 
               * Called at the start of each frame to prepare ImGui for
               * rendering. Must be called before any ImGui calls.
               */
              virtual void NewFrame() = 0;
              
              /**
               * @brief Render the GUI.
               * 
               * Finalizes the frame and renders all ImGui draw data.
               * Called after all UI code has been executed.
               */
              virtual void Render() = 0;
              
              /**
               * @brief Set the docking layout controller.
               * 
               * Registers a callback that sets up the initial docking layout
               * and configures panel positions.
               * 
               * @param controller Function to call for dock configuration
               */
              virtual void SetDockController(std::function <void()> controller) = 0;
        };
    } // namespace GUI
} // namespace Arche

#endif // I_GUISYSTEM_H