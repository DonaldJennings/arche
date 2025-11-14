#pragma once

#include <EngineCore.h>
#include <LoggingService.h>
#include <RenderingSystem.h>
#include <WorldSystem.h>

namespace Arche {
    namespace GUI {

        struct UIContext {
          private:
            std::shared_ptr<Arche::Core::EngineCore> engineCore;

          public:
            UIContext(std::shared_ptr<Arche::Core::EngineCore> engineIn) : engineCore{engineIn} {};

            std::shared_ptr<Arche::Core::LoggingService> logger() const { return engineCore->getLogger(); }

            std::shared_ptr<Arche::Render::RenderingSystem> renderer() const { return engineCore->getRenderer(); }

            std::shared_ptr<Arche::Scene::WorldSystem> worldSystem() const { return engineCore->getWorld(); }

            void pauseSimulation() { engineCore->getTimer()->pause(); };
            void runSimulation() { engineCore->getTimer()->resume(); };
            bool simulationIsPaused() const { return engineCore->getTimer()->isPaused(); };

            void toggleSimulationState() {

                if (simulationIsPaused()) {
                    engineCore->getTimer()->resume();
                } else {
                    engineCore->getTimer()->pause();
                }
            };

            Core::GlobalSettings &globalSettings() const { return engineCore->getGlobalSettings(); };
        };
    } // namespace GUI
} // namespace Arche