#pragma once

#include <EngineCore.h>
#include <LoggingService.h>
#include <RenderingSystem.h>
#include <WorldSystem.h>

#include <cstdint>
#include <optional>
#include <utility>

namespace Arche {
    namespace GUI {

        /**
         * @brief Editor session state and engine interface.
         * 
         * EditorSession acts as the bridge between the editor UI and the
         * engine core. It provides convenient access to engine subsystems
         * and maintains editor-specific state like entity selection and
         * hover information.
         * 
         * This class is the primary way UI panels interact with the engine,
         * providing a clean interface without exposing low-level engine details.
         */
        class EditorSession {
          public:
            /**
             * @brief Construct an editor session.
             * 
             * @param engineIn Shared pointer to the engine core instance
             */
            explicit EditorSession(std::shared_ptr<Core::EngineCore> engineIn) : engineCore{std::move(engineIn)} {}

            /**
             * @brief Get the logging service.
             * @return Shared pointer to logger, or nullptr if engine not initialized
             */
            auto logger() const { return engineCore ? engineCore->getLogger() : nullptr; }
            
            /**
             * @brief Get the rendering system.
             * @return Shared pointer to renderer, or nullptr if engine not initialized
             */
            auto renderer() const { return engineCore ? engineCore->getRenderer() : nullptr; }
            
            /**
             * @brief Get the world system.
             * @return Shared pointer to world, or nullptr if engine not initialized
             */
            auto worldSystem() const { return engineCore ? engineCore->getWorld() : nullptr; }
            
            /**
             * @brief Get the global settings.
             * @return Reference to global settings
             */
            auto &globalSettings() const { return engineCore->getGlobalSettings(); }

            /**
             * @brief Check if simulation is paused.
             * @return true if paused or not running, false if running
             */
            bool simulationIsPaused() const {
                return !engineCore || engineCore->simulationIsPaused();
            }

            /**
             * @brief Toggle simulation between running and paused.
             */
            void toggleSimulationState() {
                if (engineCore) {
                    engineCore->toggleSimulation();
                }
            }

            /**
             * @brief Reset simulation to initial state.
             */
            void resetSimulationState() {
                if (engineCore) {
                    engineCore->resetSimulation();
                }
            }

            /**
             * @brief Set the currently selected entity.
             * @param id Entity ID to select
             */
            void setSelectedEntity(std::optional<uint64_t> id) { selectedEntityID = id; }
            
            /**
             * @brief Clear entity selection.
             */
            void clearSelectedEntity() { selectedEntityID.reset(); }
            
            /**
             * @brief Get the currently selected entity.
             * @return Optional entity ID
             */
            std::optional<uint64_t> getSelectedEntity() const { return selectedEntityID; }

            /**
             * @brief Set the currently hovered entity.
             * 
             * Used for mouse-over highlighting and tooltips.
             * 
             * @param id Entity ID being hovered
             */
            void setHoveredEntity(std::optional<uint64_t> id) { hoveredEntityID = id; }
            
            /**
             * @brief Clear entity hover state.
             */
            void clearHoveredEntity() { hoveredEntityID.reset(); }
            
            /**
             * @brief Get the currently hovered entity.
             * @return Optional entity ID
             */
            std::optional<uint64_t> getHoveredEntity() const { return hoveredEntityID; }

          private:
            std::shared_ptr<Core::EngineCore> engineCore;       ///< Engine instance
            std::optional<std::uint64_t> selectedEntityID;      ///< Selected entity ID
            std::optional<std::uint64_t> hoveredEntityID;       ///< Hovered entity ID
        };

    } // namespace GUI
} // namespace Arche