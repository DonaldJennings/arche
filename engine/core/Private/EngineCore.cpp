#include "EngineCore.h"

#include "MaterialLoader.h"
#include "MeshGenerator.h"
#include "GeometryPass.h"
#include "SkyPass.h"
#include "DebugPass.h"
#include "ShaderLoader.h"

#include <memory> // Ensure this is included for std::shared_ptr

#include <glm/common.hpp>
#include <glm/glm.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

    std::filesystem::path GetExecutablePath() {
#if defined(_WIN32)
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        return std::filesystem::path(buffer);
#elif defined(__APPLE__)
        char buffer[1024];
        uint32_t size = sizeof(buffer);
        _NSGetExecutablePath(buffer, &size);
        return std::filesystem::canonical(buffer);
#else // Linux
        char buffer[1024];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        buffer[len] = '\0';
        return std::filesystem::canonical(buffer);
#endif
    }
} // namespace

namespace Arche {
    namespace Core {
        void EngineCore::registerSubsystem(std::shared_ptr<ISubsystem> newSubsystem) {
            subsystems.emplace_back(std::move(newSubsystem));
        }

        void EngineCore::initialise() {
            worldSystem->setPhysics(physicsSystem);
            renderingSystem->initialise();

            // --- Setup camera ---
            auto camera = std::make_shared<Render::Camera>();
            camera->SetPosition({0.0, 2.0, 5.0});
            camera->setPerspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
            renderingSystem->setMainCamera(camera);

            // --- R
            // esolve asset directory next to executable ---
            auto exePath = GetExecutablePath();
            auto exeDir = exePath.parent_path();
            std::filesystem::path assetRoot = exeDir / "assets";

            auto &resources = renderingSystem->resources();

            // --- Loaders ---
            Render::ShaderLoader shaderLoader(resources, assetRoot);
            Render::MaterialLoader materialLoader(resources, assetRoot);

            // ======================================================
            // 1. Load Shader Techniques
            // ======================================================
            shaderLoader.load("shaders/flat/flat.shader");
            shaderLoader.load("shaders/blinn-phong/blinn-phong.shader");
            shaderLoader.load("shaders/pbr/pbr.shader");
            shaderLoader.load("shaders/sky/sky.shader");
            shaderLoader.load("shaders/debug-lines/debug-lines.shader");

            // ======================================================
            // 2. Load Materials (.mat files)
            // ======================================================
            materialLoader.load("materials/cube.mat");
            materialLoader.load("materials/uv_sphere.mat");
            materialLoader.load("materials/plane.mat");
            materialLoader.load("materials/particle.mat");

            // ======================================================
            // 3. Register Procedural Meshes (from MeshGenerator)
            // ======================================================
            resources.registerMesh(Render::MeshGenerator::makeUvSphere("uv_sphere.mesh", 1.0f, 32, 16));
            resources.registerMesh(Render::MeshGenerator::makeCube("cube.mesh", 1.0f));
            resources.registerMesh(Render::MeshGenerator::makePlane("plane.mesh", 10.0f, 10.0f, 10, 10));
            resources.registerMesh(Render::MeshGenerator::makePointSphere("particle.mesh")); // optional

            renderingSystem->addRenderPass(std::make_shared<Render::SkyPass>());
            renderingSystem->addRenderPass(std::make_shared<Render::GeometryPass>());
            renderingSystem->addRenderPass(std::make_shared<Render::DebugPass>());
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