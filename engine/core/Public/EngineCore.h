#ifndef ARCHE_CORE_ENGINECORE_H
#define ARCHE_CORE_ENGINECORE_H

#include <memory>
#include <vector>

#include <ISubsystem.h>
#include <JobPoolService.h>
#include <LoggingService.h>
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
            EngineCore() : timingService(std::make_shared<TimingService>()), loggerService(std::make_shared<LoggingService>()), jobPoolService(std::make_shared<JobPoolService>()) {}
            
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
            void mainLoop();

            /**
             * @brief Shuts down the engine core and releases resources.
             */
            void shutdown();

            // Getters for services accessible to subsystems
            std::shared_ptr<TimingService> getTimingService() const { return timingService; }
            std::shared_ptr<LoggingService> getLoggingService() const { return loggerService; }
            std::shared_ptr<JobPoolService> getJobPoolService() const { return jobPoolService; }

          private:
            std::vector<std::shared_ptr<ISubsystem>> subsystems;
            std::shared_ptr<TimingService> timingService;
            std::shared_ptr<LoggingService> loggerService;
            std::shared_ptr<JobPoolService> jobPoolService;
            bool isRunning{false};
        };
    } // namespace Core
} // namespace Arche
#endif // ARCHE_CORE_ENGINECORE_H