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
#include <ShadowPass.h>

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
            if (timingService) {
                timingService->stop();
            }
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

            renderingSystem->addRenderPass(std::make_shared<Render::ShadowPass>());
            renderingSystem->addRenderPass(std::make_shared<Render::SkyPass>());
            renderingSystem->addRenderPass(std::make_shared<Render::GeometryPass>());
            renderingSystem->addRenderPass(std::make_shared<Render::DebugPass>());

            simulationState = SimulationState::Idle;
            simulationSnapshotCaptured = false;

            ARCHE_LOG_INFO(loggingService, "Engine Core Initialised.");
            if (timingService) {
                timingService->stop();
                timingService->tick(); // Initial tick to set start time
            }
        }

        void EngineCore::tick() {
            double dt = 0.0;
            if (timingService) {
                timingService->tick();
                dt = timingService->deltaTime();
            }

            const double simulationDt = simulationState == SimulationState::Running ? dt : 0.0;
            if (worldSystem) {
                worldSystem->update(simulationDt);
            }

            // Build render scene and render
            if (renderingSystem && worldSystem) {
                auto scene = worldSystem->buildRenderScene();
                renderingSystem->render(scene, globalSettings->getRenderSettings());
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

        void EngineCore::toggleSimulation() {
            if (simulationState == SimulationState::Running) {
                pauseSimulation();
            } else {
                playSimulation();
            }
        }

        void EngineCore::playSimulation() {
            if (!worldSystem || !timingService)
                return;

            if (simulationState == SimulationState::Running)
                return;

            if (!simulationSnapshotCaptured) {
                worldSystem->saveInitialState();
                simulationSnapshotCaptured = true;
            }

            if (simulationState == SimulationState::Idle) {
                timingService->start();
            } else if (simulationState == SimulationState::Paused) {
                timingService->resume();
            }

            simulationState = SimulationState::Running;
        }

        void EngineCore::pauseSimulation() {
            if (!timingService)
                return;

            if (simulationState != SimulationState::Running)
                return;

            timingService->pause();
            simulationState = SimulationState::Paused;
        }

        void EngineCore::resetSimulation() {
            if (timingService) {
                timingService->stop();
            }

            if (simulationSnapshotCaptured && worldSystem) {
                worldSystem->restoreInitialState();
                worldSystem->clearInitialState();
            }

            simulationSnapshotCaptured = false;
            simulationState = SimulationState::Idle;
        }

    } // namespace Core
} // namespace Arche