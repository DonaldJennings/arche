#include "EngineCore.h"

#include "EngineCore.h"
#include <memory> // Ensure this is included for std::shared_ptr

namespace Arche {
    namespace Core {
        void EngineCore::registerSubsystem(std::shared_ptr<ISubsystem> newSubsystem) {
            subsystems.emplace_back(std::move(newSubsystem));
        }

        void EngineCore::initialise(GLFWwindow *externalWindow) {
            for (const std::shared_ptr<ISubsystem> &subsystem : subsystems) {
                subsystem->initialise();
            }

            renderingSystem->initialise(externalWindow);
        }

        void EngineCore::update() {

            // Update timing
            timingService->tick();

            // Update all subsystems
            for (const std::shared_ptr<ISubsystem> &subsystem : subsystems) {
                subsystem->update(timingService->deltaTime());
            }

            // Render the scene
            renderingSystem->render(worldSystem->getWorld()->view().bodies);
        }

        void EngineCore::shutdown() {
            for (const std::shared_ptr<ISubsystem> &subsystem : subsystems) {
                subsystem->shutdown();
            }

            renderingSystem->shutdown();
        }
    } // namespace Core
} // namespace Arche