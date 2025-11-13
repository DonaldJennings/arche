#include "EngineCore.h"

#include "EngineCore.h"
#include <memory> // Ensure this is included for std::shared_ptr

namespace Arche {
    namespace Core {
        void EngineCore::registerSubsystem(std::shared_ptr<ISubsystem> newSubsystem) {
            subsystems.emplace_back(std::move(newSubsystem));
        }

        void EngineCore::initialise(GLFWwindow *externalWindow) {
            worldSystem->setPhysics(physicsSystem);
            renderingSystem->initialise();

            auto camera{std::make_shared<Scene::Camera>()};
            camera->setPerspective(60.0, 16.0 / 9.0, 0.1, 1000.0);
            renderingSystem->setMainCamera(camera);
        }

        void EngineCore::update() {

            // Update timing
            timingService->tick();

            // Update all subsystems
            for (const std::shared_ptr<ISubsystem> &subsystem : subsystems) {
                subsystem->update(timingService->deltaTime());
            }

            // Render the scene
            renderingSystem->render(*worldSystem);
        }

        void EngineCore::shutdown() {
            for (const std::shared_ptr<ISubsystem> &subsystem : subsystems) {
                subsystem->shutdown();
            }

            renderingSystem->shutdown();
        }
    } // namespace Core
} // namespace Arche