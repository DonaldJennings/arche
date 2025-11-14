#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
        m_initialised = m_shadowMapID != 0;
    }

    void Arche::Render::ShadowPass::render(const RenderView &view, IRenderBackend &backend, const Camera &camera,
                                           ResourceRegistry &resources, RenderPassSettings &settings) {
        settings.shadowSettings.enabled = false;
        settings.shadowSettings.shadowMapID = 0;
        settings.shadowSettings.resolution = 0;
        settings.shadowSettings.lightSpaceMatrix = glm::mat4(1.0f);

        if (!m_initialised)
            return;

        auto depthShader = resources.getShader("ShadowDepth");
        if (!depthShader)
            return;

        glm::vec3 lightDir = glm::normalize(settings.globalSettings.directionalLightDirection);

        glm::vec3 lightPos = -lightDir * settings.globalSettings.directionalLightDistance;

        float orthoRange = 100.0f;
        glm::mat4 lightProj{glm::ortho(-orthoRange, orthoRange, -orthoRange, orthoRange, 1.0f, 200.0f)};
        glm::mat4 lightView{glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f))};

        m_lightSpaceMatrix = lightProj * lightView;

        backend.beginShadowPass();
        backend.setShader(depthShader);
        backend.setUniformMat4("uLightSpaceMatrix", m_lightSpaceMatrix);

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

            backend.setUniformMat4("uModel", model);

            backend.drawMeshDepthOnly(*mesh, model);
        }

        backend.endShadowPass();

        m_shadowMapID = backend.getShadowMapTextureID();
        m_initialised = m_shadowMapID != 0;

        if (!m_initialised)
            return;

        settings.shadowSettings.shadowMapID = m_shadowMapID;
        settings.shadowSettings.lightSpaceMatrix = m_lightSpaceMatrix;
        settings.shadowSettings.resolution = m_resolution;
        settings.shadowSettings.enabled = true;
    }
} // namespace Arche::Render
