#include "EngineCore.h"
#include "WorldSystem.h"
#include "RenderingSystem.h"
#include "PhysicsSystem.h"
#include "OpenGLBackend.h"
#include "ShaderLoader.h"
#include "MaterialLoader.h"
#include "ResourceRegistry.h"
#include "SkyPass.h"
#include "GeometryPass.h"
#include "DebugPass.h"

#include <filesystem>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {
    // Returns the directory containing the application executable.
    std::filesystem::path GetExecutableDir() {
#ifdef _WIN32
        wchar_t buf[MAX_PATH];
        DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH);
        if (len == 0)
            return std::filesystem::current_path();
        return std::filesystem::path(buf).parent_path();
#else
        char buf[4096];
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (len != -1) {
            buf[len] = 0;
            return std::filesystem::path(buf).parent_path();
        }
        return std::filesystem::current_path();
#endif
    }
}

namespace Arche {
    namespace Core {

        EngineCore::EngineCore() {
            loggingService = std::make_shared<LoggingService>();
            timingService = std::make_shared<TimingService>(loggingService);
            globalSettings = std::make_shared<GlobalSettings>();
            assetsPath = (GetExecutableDir() / "assets").string();
        }

        EngineCore::~EngineCore() {}

        void EngineCore::initialise() {
            ARCHE_LOG_INFO(loggingService, "Engine Core Initialising...");

            physicsSystem = std::make_shared<Physics::PhysicsSystem>(loggingService, globalSettings);
            physicsSystem->initialise();

            worldSystem = std::make_shared<Scene::WorldSystem>(loggingService, timingService);
            worldSystem->setPhysics(physicsSystem);
            worldSystem->initialise();

            auto backend = std::make_unique<Render::OpenGLBackend>();
            renderingSystem = std::make_shared<Render::RenderingSystem>(loggingService, std::move(backend));
            
            // Initialize loaders
            shaderLoader = std::make_shared<Render::ShaderLoader>(renderingSystem->resources(), assetsPath / "shaders");
            materialLoader = std::make_shared<Render::MaterialLoader>(renderingSystem->resources(), assetsPath / "materials");            

            renderingSystem->initialise(shaderLoader, materialLoader);

            renderingSystem->addRenderPass(std::make_shared<Render::SkyPass>());
            renderingSystem->addRenderPass(std::make_shared<Render::GeometryPass>());
            renderingSystem->addRenderPass(std::make_shared<Render::DebugPass>());

            ARCHE_LOG_INFO(loggingService, "Engine Core Initialised.");
            timingService->tick(); // Initial tick to set start time
        }

        void EngineCore::tick() {
            timingService->tick();
            double dt = timingService->deltaTime();

            worldSystem->update(dt);

            // Render the world
            if (renderingSystem) {
                renderingSystem->render(*worldSystem, globalSettings->getRenderSettings());
            }
        }

        void EngineCore::shutdown() {
            ARCHE_LOG_INFO(loggingService, "Engine Core Shutting Down...");
            if (renderingSystem)
                renderingSystem->shutdown();
            if (worldSystem)
                worldSystem->shutdown();
            if (physicsSystem)
                physicsSystem->shutdown();
            ARCHE_LOG_INFO(loggingService, "Engine Core Shutdown Complete.");
        }

    } // namespace Core
} // namespace Arche