#pragma once

#include <EngineCore.h>
#include <LoggingService.h>
#include <RenderingSystem.h>
#include <WorldSystem.h>

namespace Arche {
    namespace GUI {

        struct UIContext {
          private:
            enum class SimulationState { Idle, Running, Paused };

            std::shared_ptr<Core::EngineCore> engineCore;
            std::optional<std::uint64_t> selectedEntityID;
            std::optional<std::uint64_t> hoveredEntityID;

            SimulationState m_simulationState{SimulationState::Idle};
            bool m_hasInitialSnapshot{false};

          public:
            UIContext(std::shared_ptr<Core::EngineCore> engineIn) : engineCore{engineIn} {}

            auto logger() const { return engineCore->getLogger(); }
            auto renderer() const { return engineCore->getRenderer(); }
            auto worldSystem() const { return engineCore->getWorld(); }
            auto &globalSettings() const { return engineCore->getGlobalSettings(); }

            bool simulationIsPaused() const { return m_simulationState != SimulationState::Running; }

            void toggleSimulationState() {
                auto timer = engineCore->getTimer();
                auto world = engineCore->getWorld();
                if (!timer || !world) {
                    return;
                }

                switch (m_simulationState) {
                case SimulationState::Idle:
                    if (!m_hasInitialSnapshot) {
                        world->saveInitialState();
                        m_hasInitialSnapshot = true;
                    }
                    timer->reset();
                    m_simulationState = SimulationState::Running;
                    break;
                case SimulationState::Running:
                    timer->pause();
                    m_simulationState = SimulationState::Paused;
                    break;
                case SimulationState::Paused:
                    timer->resume();
                    m_simulationState = SimulationState::Running;
                    break;
                }
            }

            void resetSimulationState() {
                auto timer = engineCore->getTimer();
                auto world = engineCore->getWorld();
                if (!timer || !world) {
                    return;
                }

                timer->reset();
                timer->pause();

                if (m_hasInitialSnapshot) {
                    world->restoreInitialState();
                }

                m_simulationState = SimulationState::Idle;
                m_hasInitialSnapshot = false;
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