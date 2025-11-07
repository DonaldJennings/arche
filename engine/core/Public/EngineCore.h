#ifndef ARCHE_CORE_ENGINECORE_H
#define ARCHE_CORE_ENGINECORE_H

#include <memory>
#include <vector>

#include "WorldSystem.h"
#include <ISubsystem.h>
#include <JobPoolService.h>
#include <LoggingService.h>
#include <Renderer.h>
#include <TimingService.h>

namespace Arche {
    namespace Core {
        /**
         * @brief Core engine class that manages subsystems and services.
         */
        class EngineCore {
          public:
            /**
             * @brief Constructs the main engine core which manages subsystems and services.
             */
            EngineCore(std::shared_ptr<Scene::World> worldIn)
                : worldSystem(std::make_shared<Scene::WorldSystem>(worldIn, loggerService,
                                                                   jobPoolService, timingService)),
                  timingService(std::make_shared<TimingService>(loggerService)), loggerService(std::make_shared<LoggingService>()),
                  jobPoolService(std::make_shared<JobPoolService>()),
                  renderingSystem(std::make_shared<Render::Renderer>(loggerService)) 
            {
                // Register the WorldSystem as a subsystem
                registerSubsystem(worldSystem);
            }

            /**
             * @brief Registers a new subsystem with the engine core.
             * @param newSubsystem
             */
            void registerSubsystem(std::shared_ptr<ISubsystem> newSubsystem);

            /**
             * @brief Initializes internal state and resources required before using the module or library.
             */
            void initialise(GLFWwindow *externalWindow);

            /**
             * @brief Runs the program's main loop.
             */
            void update();

            /**
             * @brief Shuts down the engine core and releases resources.
             */
            void shutdown();

            // Getters for services accessible to subsystems
            std::shared_ptr<TimingService> getTimingService() const { return timingService; }
            std::shared_ptr<LoggingService> getLoggingService() const { return loggerService; }
            std::shared_ptr<JobPoolService> getJobPoolService() const { return jobPoolService; }
            std::shared_ptr<Render::Renderer> getRenderer() const { return renderingSystem; }
            std::shared_ptr<Scene::WorldSystem> getWorld() const { return worldSystem; }

          private:
            std::shared_ptr<LoggingService> loggerService;
            std::shared_ptr<TimingService> timingService;
            std::shared_ptr<JobPoolService> jobPoolService;
            std::shared_ptr<Arche::Scene::WorldSystem> worldSystem;
            std::shared_ptr<Render::Renderer> renderingSystem;
            std::vector<std::shared_ptr<ISubsystem>> subsystems;
            bool isRunning{false};
        };
    } // namespace Core
} // namespace Arche
#endif // ARCHE_CORE_ENGINECORE_H