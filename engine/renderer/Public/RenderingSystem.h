#pragma once

#include <cstdint>
#include "Camera.h"
#include "IRenderBackend.h"
#include "IRenderPass.h"
#include "LoggingService.h"
#include "MaterialLoader.h"
#include "MeshGenerator.h"
#include "RenderScene.h"
#include "ResourceRegistry.h"
#include "ShaderLoader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <sstream>
#include <string_view>

namespace Arche {
    namespace Render {

        class RenderingSystem {
          public:
            RenderingSystem(std::shared_ptr<Core::LoggingService> logger,
                            std::unique_ptr<IRenderBackend> backend,
                            glm::ivec2 backBufferSize = {800, 600},
                            bool vsync = true)
                : m_logger(logger), m_backend(std::move(backend)),
                  m_backBufferSize(backBufferSize), m_vsync(vsync) {}

            void initialise(std::shared_ptr<ShaderLoader> shaderLoader,
                            std::shared_ptr<MaterialLoader> materialLoader) {
                shaderLoader->loadAllInDirectory();
                materialLoader->loadAllInDirectory();

                m_resources.registerMesh(MeshGenerator::makeUvSphere("uv_sphere.mesh", 1.0f, 32, 32));
                m_resources.registerMesh(MeshGenerator::makeCube("cube.mesh", 1.0f));
                m_resources.registerMesh(MeshGenerator::makePlane("plane.mesh", 10.0f, 10.0f, 10, 10));
                m_resources.registerMesh(MeshGenerator::makePointSphere("point_sphere.mesh"));

                if (m_backend) {
                    m_backend->initialise();
                    m_backend->resize(m_backBufferSize);
                    m_backend->uploadSceneGeometry(m_resources); // F-14: flat geometry SSBOs
                }
            }

            void shutdown() noexcept {
                if (m_backend) m_backend->shutdown();
            }

            void onResize(glm::ivec2 newSize) {
                m_backBufferSize = newSize;
                if (m_backend) m_backend->resize(newSize);
                if (m_mainCamera) {
                    double aspectRatio = static_cast<double>(newSize.x) /
                                        static_cast<double>(newSize.y);
                    m_mainCamera->setPerspective(60.0, aspectRatio, 0.1, 1000.0);
                }
            }

            void setMainCamera(std::shared_ptr<Arche::Render::Camera> camera) {
                m_mainCamera = std::move(camera);
            }
            std::shared_ptr<Arche::Render::Camera> getMainCamera() const { return m_mainCamera; }

            void addRenderPass(std::shared_ptr<IRenderPass> pass) {
                if (pass) {
                    pass->initialise(*m_backend);
                    m_passes.push_back(std::move(pass));
                }
            }

            ResourceRegistry       &resources()       { return m_resources; }
            const ResourceRegistry &resources() const { return m_resources; }

            /**
             * @brief Render one frame from the given RenderScene.
             *
             * @param scene   Scene data built by WorldSystem this frame.
             * @param settings Global render settings.
             */
            void render(const RenderScene &scene, const Core::RenderSettings &settings) {
                if (!m_backend) {
                    ARCHE_LOG_ERROR(m_logger, "Rendering backend not set, cannot render frame.");
                    return;
                }
                if (!m_mainCamera) {
                    ARCHE_LOG_ERROR(m_logger, "Main camera not set, cannot render frame.");
                    return;
                }

                RenderPassSettings renderingSettings;
                renderingSettings.globalSettings = settings;

                RenderView viewData;
                viewData.viewMatrix       = glm::mat4(m_mainCamera->GetViewMatrix());
                viewData.projectionMatrix = glm::mat4(m_mainCamera->GetProjectionMatrix());
                viewData.cameraPosition   = m_mainCamera->GetPosition();
                viewData.opaqueObjects    = scene.opaqueObjects;

                m_backend->setWireframe(settings.wireframe);
                m_backend->updateInstanceBuffer(scene); // F-14: per-instance data before frame
                m_backend->beginFrame();

                for (const std::shared_ptr<IRenderPass> &pass : m_passes)
                    pass->render(viewData, *m_backend, *m_mainCamera, m_resources, renderingSettings);
            }

            /**
             * @brief Submit the current frame and present it.
             *
             * Must be called after render() and after the GUI system has called
             * ImGui::Render() so that draw data is available for the ImGui pass.
             * For OpenGL this is a no-op (swap buffers is handled externally).
             */
            void present() {
                if (m_backend) m_backend->endFrame();
            }

            glm::ivec2   getBackBufferSize()      const { return m_backBufferSize; }
            uint64_t     getRenderTextureID()     const {
                return m_backend ? m_backend->getRenderTextureID() : 0u;
            }
            bool         needsRenderTextureYFlip() const {
                return m_backend ? m_backend->needsRenderTextureYFlip() : true;
            }

            /**
             * @brief Return a raw (non-owning) pointer to the underlying render backend.
             *
             * Callers may dynamic_cast to a concrete type (e.g. VulkanBackend) when
             * the backend is known at compile time.  Ownership stays with RenderingSystem.
             */
            IRenderBackend *getBackend() const { return m_backend.get(); }

          private:
            std::vector<std::shared_ptr<Render::IRenderPass>> m_passes;
            std::unique_ptr<IRenderBackend>                    m_backend;
            std::shared_ptr<Core::LoggingService>              m_logger;
            std::shared_ptr<Arche::Render::Camera>             m_mainCamera;
            Arche::Render::RenderPassSettings                  m_renderPassSettings;
            ResourceRegistry                                   m_resources;

            glm::ivec2 m_backBufferSize;
            bool       m_vsync;
        };

    } // namespace Render
} // namespace Arche
