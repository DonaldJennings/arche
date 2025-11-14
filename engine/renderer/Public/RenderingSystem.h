#pragma once

#include "Camera.h"
#include "IRenderBackend.h"
#include "LoggingService.h"
#include "ResourceRegistry.h"
#include "WorldSystem.h"
#include "ShaderLoader.h"
#include "MaterialLoader.h"

#include "IRenderPass.h"
#include "MeshGenerator.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <sstream>
#include <string_view>

namespace Arche {
    namespace Render {
        class RenderingSystem {
          public:
            RenderingSystem(std::shared_ptr<Core::LoggingService> logger, std::unique_ptr<IRenderBackend> backend,
                            glm::ivec2 backBufferSize = {800, 600}, bool vsync = true)
                : m_logger(logger), m_backend(std::move(backend)), m_backBufferSize(backBufferSize), m_vsync(vsync) {}

            void initialise(std::shared_ptr<ShaderLoader> shaderLoader, std::shared_ptr<MaterialLoader> materialLoader) {

                shaderLoader->loadAllInDirectory();
                materialLoader->loadAllInDirectory();

                m_resources.registerMesh(MeshGenerator::makeUvSphere("uv_sphere.mesh", 1.0f, 32, 32));
                m_resources.registerMesh(MeshGenerator::makeCube("cube.mesh", 1.0f));
                m_resources.registerMesh(MeshGenerator::makePlane("plane.mesh", 10.0f, 10.0f, 10, 10));
                m_resources.registerMesh(MeshGenerator::makePointSphere("point_sphere.mesh"));

                // Load shaders, materials, meshes here if needed

                if (m_backend) {
                    m_backend->initialise();
                    m_backend->resize(m_backBufferSize);
                }
            }

            void shutdown() noexcept {
                if (m_backend) {
                    m_backend->shutdown();
                }
            }

            void onResize(glm::ivec2 newSize) {
                m_backBufferSize = newSize;
                if (m_backend)
                    m_backend->resize(newSize);
                if (m_mainCamera) {
                    double aspectRatio = static_cast<double>(newSize.x) / static_cast<double>(newSize.y);
                    m_mainCamera->setPerspective(60.0, aspectRatio, 0.1, 1000.0);
                }
            }

            void setMainCamera(std::shared_ptr<Arche::Render::Camera> camera) { m_mainCamera = std::move(camera); }
            std::shared_ptr<Arche::Render::Camera> getMainCamera() const { return m_mainCamera; }

            void addRenderPass(std::shared_ptr<IRenderPass> pass) {
                if (pass) {
                    pass->initialise(*m_backend);
                    m_passes.push_back(std::move(pass));
                }
            }

            ResourceRegistry &resources() { return m_resources; }
            const ResourceRegistry &resources() const { return m_resources; }

            void render(const Scene::WorldSystem &world, const Core::RenderSettings& settings) {
                if (!m_backend) {
                    ARCHE_LOG_ERROR(m_logger, "Rendering backend not set, cannot render frame.");
                    return;
                }

                if (!m_mainCamera) {
                    ARCHE_LOG_ERROR(m_logger, "Main camera not set, cannot render frame.");
                    return;
                }

                glm::mat4 view = glm::mat4(m_mainCamera->GetViewMatrix());
                glm::mat4 projection = glm::mat4(m_mainCamera->GetProjectionMatrix());

                m_backend->beginFrame();

                for (std::shared_ptr<IRenderPass> pass : m_passes) {

                    RenderView viewData;
                    viewData.viewMatrix = view;
                    viewData.projectionMatrix = projection;
                    viewData.cameraPosition = m_mainCamera->GetPosition();

                    // Collect opaque and transparent objects from the world
                    for (const auto &entity : world.view().bodies) {
                        // Here you would check entity's material properties to decide which list to add to
                        // For simplicity, we add all to opaqueObjects
                        viewData.opaqueObjects.push_back(entity);
                    }

                    pass->render(viewData, *m_backend, *m_mainCamera, m_resources, settings);
                }
                m_backend->endFrame();
            }

            glm::ivec2 getBackBufferSize() const { return m_backBufferSize; }
            unsigned int getRenderTextureID() const { return m_backend ? m_backend->getRenderTextureID() : 0u; }

          private:
            std::vector<std::shared_ptr<Render::IRenderPass>> m_passes;
            std::unique_ptr<IRenderBackend> m_backend;
            std::shared_ptr<Core::LoggingService> m_logger;
            std::shared_ptr<Arche::Render::Camera> m_mainCamera;
            ResourceRegistry m_resources;

            glm::ivec2 m_backBufferSize;
            bool m_vsync;
        };
    } // namespace Render
} // namespace Arche