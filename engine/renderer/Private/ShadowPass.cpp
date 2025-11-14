#pragma once
#include <glm/glm.hpp>
#include <memory>

#include "Camera.h"
#include "GlobalSettings.h"
#include "IRenderBackend.h"
#include "IRenderPass.h"
#include "Material.h"
#include "Mesh.h"
#include "ResourceRegistry.h"
#include "Shader.h"
#include "ShadowPass.h"

namespace Arche::Render {

    void Arche::Render::ShadowPass::initialise(IRenderBackend &backend) {
        backend.initialiseShadowResources(m_resolution);
        m_shadowMapID = backend.getShadowMapTextureID();
    }

    void Arche::Render::ShadowPass::render(const RenderView &view, IRenderBackend &backend, const Camera &camera,
                                           ResourceRegistry &resources, RenderPassSettings &settings) {
        glm::vec3 lightDir = glm::normalize(settings.globalSettings.directionalLightDirection);

        glm::vec3 lightPos = -lightDir * settings.globalSettings.directionalLightDistance;

        float orthoRange = 500.0f;
        glm::mat4 lightProj{glm::ortho(-orthoRange, orthoRange, -orthoRange, orthoRange, 0.1f, 1000.0f)};
        glm::mat4 lightView{glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f))};

        m_lightSpaceMatrix = lightProj * lightView;

        backend.beginShadowPass(lightView, lightProj);

        for (const auto &entity : view.opaqueObjects) {
            if (!entity)
                continue;

            glm::mat4 model{glm::translate(glm::mat4(1.0f), entity->getPosition())};
            model = glm::scale(model, entity->getScale());

            // We don't care about the material here: only mesh + model
            auto mesh = resources.getMesh(entity->getMeshId());
            if (!mesh) {
                continue;
            }

            backend.drawMeshDepthOnly(*mesh, model);
        }

        settings.shadowSettings.shadowMapID = m_shadowMapID;
        settings.shadowSettings.lightSpaceMatrix = m_lightSpaceMatrix;
        settings.shadowSettings.resolution = m_resolution;
        settings.shadowSettings.enabled = true;

        backend.endShadowPass();
    }
} // namespace Arche::Render
