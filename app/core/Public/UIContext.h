#pragma once

#include "Renderer.h"
#include <LoggingService.h>
#include <WorldSystem.h>
#include <EngineCore.h>

namespace Arche {
    namespace GUI {

        struct UIContext {
          private:
            std::shared_ptr<Arche::Core::EngineCore> engineCore;

          public:
            UIContext(std::shared_ptr<Arche::Core::EngineCore> engineIn) : engineCore{engineIn} {};

            std::shared_ptr<Arche::Core::LoggingService> logger() const { return engineCore->getLoggingService(); }

            std::shared_ptr<Arche::Render::RenderingSystem> renderer() const { return engineCore->getRenderer(); }

            std::shared_ptr<Arche::Scene::WorldSystem> worldSystem() const { return engineCore->getWorld(); }

            void pauseSimulation() {
                engineCore->getTimingService()->pause();
            };
            void runSimulation() {
                engineCore->getTimingService()->resume();
            };
            bool simulationIsPaused() const { return engineCore->getTimingService()->isPaused(); };

            void toggleSimulationState() {
                
                if (simulationIsPaused()) {
                    engineCore->getTimingService()->resume();
                } else {
                    engineCore->getTimingService()->pause();
                }
            };
        };
    } // namespace GUI
} // namespace Arche