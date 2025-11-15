#pragma once

#include <EngineCore.h>
#include <LoggingService.h>
#include <RenderingSystem.h>
#include <WorldSystem.h>

namespace Arche {
    namespace GUI {

        struct UIContext {
          private:
            std::shared_ptr<Core::EngineCore> engineCore;
            std::optional<std::uint64_t> selectedEntityID;
            std::optional<std::uint64_t> hoveredEntityID;

            bool m_simulationPaused = true;
            bool m_simulationHasStarted = false;

          public:
            UIContext(std::shared_ptr<Core::EngineCore> engineIn) : engineCore{engineIn} {}

            auto logger() const { return engineCore->getLogger(); }
            auto renderer() const { return engineCore->getRenderer(); }
            auto worldSystem() const { return engineCore->getWorld(); }
            auto &globalSettings() const { return engineCore->getGlobalSettings(); }

            bool simulationIsPaused() const { return m_simulationPaused; }

            void toggleSimulationState() {
                if (m_simulationPaused) {
                    if (!m_simulationHasStarted) {
                        engineCore->getWorld()->saveInitialState(); // SAVE ONLY ON FIRST PLAY
                        m_simulationHasStarted = true;
                    }

                    engineCore->getTimer()->resume();
                    m_simulationPaused = false;
                } else {
                    engineCore->getTimer()->pause();
                    m_simulationPaused = true;
                }
            }

            void resetSimulationState() {
                engineCore->getTimer()->reset();
                engineCore->getWorld()->restoreInitialState();

                m_simulationPaused = true;
                m_simulationHasStarted = false;
            }

            // Selection:
            void setSelectedEntity(std::optional<uint64_t> id) { selectedEntityID = id; }
            void clearSelectedEntity() { selectedEntityID.reset(); }
            std::optional<uint64_t> getSelectedEntity() const { return selectedEntityID; }

            // Hover:
            void setHoveredEntity(std::optional<uint64_t> id) { hoveredEntityID = id; }
            void clearHoveredEntity() { hoveredEntityID.reset(); }
            std::optional<uint64_t> getHoveredEntity() const { return hoveredEntityID; }
        };

    } // namespace GUI
} // namespace Arche