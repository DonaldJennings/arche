#ifndef ARCHE_CORE_ENGINECORE_H
#define ARCHE_CORE_ENGINECORE_H

#include <memory>
#include <vector>

#include "WorldSystem.h"
#include <ISubsystem.h>
#include <LoggingService.h>
#include <PhysicsSystem.h>
#include <TimingService.h>
#include <RenderingSystem.h>
#include <OpenGLBackend.h>

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
            EngineCore()
                : loggerService(std::make_shared<LoggingService>()),
                  physicsSystem(std::make_shared<Physics::PhysicsSystem>(loggerService)),
                  timingService(std::make_shared<TimingService>(loggerService)),
                  worldSystem(std::make_shared<Scene::WorldSystem>(loggerService, timingService))
            {
                auto backend = std::make_unique<Arche::Render::OpenGLBackend>();
                backend->setLogger(loggerService);
                renderingSystem = std::make_shared<Render::RenderingSystem>(loggerService, std::move(backend));

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
            void initialise();

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
            std::shared_ptr<Render::RenderingSystem> getRenderer() const { return renderingSystem; }
            std::shared_ptr<Scene::WorldSystem> getWorld() const { return worldSystem; }

          private:
            std::shared_ptr<LoggingService> loggerService;
            std::shared_ptr<Physics::PhysicsSystem> physicsSystem;
            std::shared_ptr<TimingService> timingService;
            std::shared_ptr<Arche::Scene::WorldSystem> worldSystem;
            std::shared_ptr<Render::RenderingSystem> renderingSystem;
            std::vector<std::shared_ptr<ISubsystem>> subsystems;
            bool isRunning{false};
        };
    } // namespace Core
} // namespace Arche
#endif // ARCHE_CORE_ENGINECORE_H