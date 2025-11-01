#include "EngineCore.h"

#include <memory> // Ensure this is included for std::shared_ptr
#include "EngineCore.h"

namespace Arche {
    namespace Core {
        void EngineCore::registerSubsystem(std::shared_ptr<ISubsystem> newSubsystem) {
            subsystems.emplace_back(std::move(newSubsystem));
        }

        void EngineCore::initialise() {
            for (const std::shared_ptr<ISubsystem>& subsystem : subsystems) {
                subsystem->initialise();
            }
            isRunning = true;
        }

        void EngineCore::mainLoop() {
            const float deltaTime = 1.0f / 60.0f; // Assuming a fixed time step for simplicity
            while (isRunning) {
                for (const std::shared_ptr<ISubsystem>& subsystem : subsystems) {
                    subsystem->update(deltaTime);
                }
            }
        }

        void EngineCore::shutdown() {
            for (const std::shared_ptr<ISubsystem>& subsystem : subsystems) {
                subsystem->shutdown();
            }
            isRunning = false;
        }
    } // namespace Core
} // namespace Arche