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

        class EditorSession {
          public:
            explicit EditorSession(std::shared_ptr<Core::EngineCore> engineIn) : engineCore{std::move(engineIn)} {}

            auto logger() const { return engineCore ? engineCore->getLogger() : nullptr; }
            auto renderer() const { return engineCore ? engineCore->getRenderer() : nullptr; }
            auto worldSystem() const { return engineCore ? engineCore->getWorld() : nullptr; }
            auto &globalSettings() const { return engineCore->getGlobalSettings(); }

            bool simulationIsPaused() const {
                return !engineCore || engineCore->simulationIsPaused();
            }

            void toggleSimulationState() {
                if (engineCore) {
                    engineCore->toggleSimulation();
                }
            }

            void resetSimulationState() {
                if (engineCore) {
                    engineCore->resetSimulation();
                }
            }

            // Selection:
            void setSelectedEntity(std::optional<uint64_t> id) { selectedEntityID = id; }
            void clearSelectedEntity() { selectedEntityID.reset(); }
            std::optional<uint64_t> getSelectedEntity() const { return selectedEntityID; }

            // Hover:
            void setHoveredEntity(std::optional<uint64_t> id) { hoveredEntityID = id; }
            void clearHoveredEntity() { hoveredEntityID.reset(); }
            std::optional<uint64_t> getHoveredEntity() const { return hoveredEntityID; }

          private:
            std::shared_ptr<Core::EngineCore> engineCore;
            std::optional<std::uint64_t> selectedEntityID;
            std::optional<std::uint64_t> hoveredEntityID;
        };

    } // namespace GUI
} // namespace Arche