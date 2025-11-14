#pragma once

#include "LoggingService.h"
#include "TimingService.h"
#include "GlobalSettings.h"
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

// Forward declarations
namespace Arche {
    namespace Scene {
        class WorldSystem;
    }
    namespace Render {
        class RenderingSystem;
        class ShaderLoader;
        class MaterialLoader;
    }
    namespace Physics {
        class PhysicsSystem;
    }
} // namespace Arche

namespace Arche {
    namespace Core {

        class EngineCore {
          public:
            EngineCore();
            ~EngineCore();

            void initialise();
            void tick();
            void shutdown();

            std::shared_ptr<Arche::Scene::WorldSystem> getWorld() const { return worldSystem; }
            std::shared_ptr<Arche::Render::RenderingSystem> getRenderer() const { return renderingSystem; }
            std::shared_ptr<Arche::Core::LoggingService> getLogger() const { return loggingService; }
            std::shared_ptr<Arche::Core::TimingService> getTimer() const { return timingService; }
            std::shared_ptr<Arche::Physics::PhysicsSystem> getPhysics() const { return physicsSystem; }
            GlobalSettings& getGlobalSettings() { return *globalSettings; }

          private:
            std::shared_ptr<Arche::Core::LoggingService> loggingService;
            std::shared_ptr<Arche::Core::TimingService> timingService;
            std::shared_ptr<Arche::Scene::WorldSystem> worldSystem;
            std::shared_ptr<Arche::Render::RenderingSystem> renderingSystem;
            std::shared_ptr<Arche::Physics::PhysicsSystem> physicsSystem;
            std::shared_ptr<GlobalSettings> globalSettings;
            std::shared_ptr<Render::ShaderLoader> shaderLoader;
            std::shared_ptr<Render::MaterialLoader> materialLoader;

            std::filesystem::path assetsPath;
        };

    } // namespace Core
} // namespace Arche