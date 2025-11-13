#pragma once

#include "Camera.h"
#include "IRenderBackend.h"
#include "ResourceRegistry.h"
#include "WorldSystem.h"
#include "LoggingService.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string_view>
#include <sstream>

namespace Arche {
    namespace Render {
        class RenderingSystem {
          public:
            RenderingSystem(std::shared_ptr<Core::LoggingService> logger, std::unique_ptr<IRenderBackend> backend,
                            glm::ivec2 backBufferSize = {800, 600}, bool vsync = true)
                : m_logger(logger), m_backend(std::move(backend)), m_backBufferSize(backBufferSize), m_vsync(vsync) {}

            void initialise() {
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

            ResourceRegistry &resources() { return m_resources; }
            const ResourceRegistry &resources() const { return m_resources; }

            void render(const Scene::WorldSystem &world) {
                if (!m_backend || !m_mainCamera)
                {
                    ARCHE_LOG_ERROR(m_logger, "Rendering backend or main camera not set, cannot render frame.");
                    return;
                }

                glm::mat4 view = glm::mat4(m_mainCamera->GetViewMatrix());
                glm::mat4 projection = glm::mat4(m_mainCamera->GetProjectionMatrix());

                m_backend->beginFrame();
                m_backend->setViewProjection(view, projection);

                auto vw = world.view();
                for (const std::shared_ptr<Scene::IEntity> &entity : vw.bodies) {
                    if (!entity) {
                        ARCHE_LOG_WARNING(m_logger, "Skipping null entity while rendering.");
                        continue;
                    }

                    auto mesh = m_resources.getMesh(entity->getMeshId());
                    auto material = m_resources.getMaterial(entity->getMaterialId());
                    if (!mesh || !material) {
                        std::ostringstream logOss;
                        logOss << "Skipping entity due to missing mesh or material: " << entity->getName() << ":" << entity->getID();
                        ARCHE_LOG_WARNING(m_logger, logOss.str());
                        continue;
                    }

                    if (auto shader = material->getShader()) {
                        m_backend->setShader(shader);
                    }
                    m_backend->setMaterial(*material);

                    glm::mat4 model = glm::translate(glm::mat4(1.0f), entity->getPosition());
                    model = glm::scale(model, entity->getScale());

                    m_backend->drawMesh(*mesh, model);
                }

                m_backend->endFrame();
            }

            glm::ivec2 getBackBufferSize() const { return m_backBufferSize; }
            unsigned int getRenderTextureID() const { return m_backend ? m_backend->getRenderTextureID() : 0u; }

          private:
            std::unique_ptr<IRenderBackend> m_backend;
            std::shared_ptr<Core::LoggingService> m_logger;
            std::shared_ptr<Arche::Render::Camera> m_mainCamera;
            ResourceRegistry m_resources;

            glm::ivec2 m_backBufferSize;
            bool m_vsync;
        };
    } // namespace Render
} // namespace Arche