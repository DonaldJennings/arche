#pragma once

#include <WorldSystem.h>
#include <LoggingService.h>

namespace Arche {
    namespace GUI {

        struct UIContext
        {
          private:
            bool isPaused{true};

          public:
            void pauseSimulation() { 
                isPaused = true; 
                if (logger) ARCHE_LOG_INFO(logger, "Simulation paused"); 
            };
            void runSimulation() { 
                isPaused = false; 
                if (logger) ARCHE_LOG_INFO(logger, "Simulation started"); 
            };
            bool simulationIsPaused() const { return isPaused; };
            void toggleSimulationState() { 
                isPaused = !isPaused; 
                if (logger) ARCHE_LOG_INFO(logger, isPaused ? "Simulation paused" : "Simulation started"); 
            };

            std::shared_ptr<Arche::Scene::WorldSystem> worldSystem;
            std::shared_ptr<Arche::Core::LoggingService> logger;

            UIContext(
                std::shared_ptr<Arche::Core::LoggingService> loggerIn,
                std::shared_ptr<Arche::Scene::WorldSystem> worldSystemIn)
                : logger(std::move(loggerIn)), worldSystem(std::move(worldSystemIn)) {}
        };
    } // namespace GUI
} // namespace Arche