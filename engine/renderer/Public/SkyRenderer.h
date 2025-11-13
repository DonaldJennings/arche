#pragma once
#include <glm/glm.hpp>
#include <memory>

#include "Camera.h"
#include "IRenderBackend.h"
#include "Material.h"
#include "Mesh.h"
#include "ResourceRegistry.h"
#include "Shader.h"

namespace Arche::Render {

    class SkyRenderer {
      public:
        SkyRenderer(ResourceRegistry &registry);

        void setShader(std::shared_ptr<Shader> shaderIn) { m_shader = shaderIn; };
        void setSunDirection(const glm::vec3 &dir) { m_sunDir = glm::normalize(dir); }
        void setSunColor(const glm::vec3 &col) { m_sunColor = col; }

        void render(IRenderBackend &backend, const Camera &cam);

      private:
        ResourceRegistry &m_registry;

        std::shared_ptr<Mesh> m_cubeMesh;
        std::shared_ptr<Shader> m_shader;
        std::shared_ptr<Material> m_material;

        glm::vec3 m_sunDir = glm::normalize(glm::vec3(0.3f, 0.8f, 0.2f));
        glm::vec3 m_sunColor = glm::vec3(5.0f, 5.0f, 4.8f);
    };

} // namespace Arche::Render
