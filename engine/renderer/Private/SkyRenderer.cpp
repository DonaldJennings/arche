#include "SkyRenderer.h"
#include <glm/gtc/matrix_inverse.hpp>

namespace Arche::Render {

    SkyRenderer::SkyRenderer(ResourceRegistry &registry) : m_registry(registry) {

        // Create a cube mesh if not already present
        m_cubeMesh = m_registry.getMesh("skybox.cube");
        if (!m_cubeMesh) {
            auto mesh = std::make_shared<Mesh>("skybox.cube");
            mesh->setPrimitive(Mesh::Primitive::Triangles);

            // Simple cube vertices in clip-space corners (-1…1)
            // Only positions needed
            std::vector<Mesh::Vertex> verts = {{{-1, -1, -1}}, {{1, -1, -1}}, {{1, 1, -1}}, {{-1, 1, -1}},
                                               {{-1, -1, 1}},  {{1, -1, 1}},  {{1, 1, 1}},  {{-1, 1, 1}}};

            uint32_t idx[] = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 0, 4, 7, 7, 3, 0,
                              1, 5, 6, 6, 2, 1, 3, 2, 6, 6, 7, 3, 0, 1, 5, 5, 4, 0};

            mesh->vertices() = verts;
            mesh->indices().assign(std::begin(idx), std::end(idx));

            m_registry.registerMesh(mesh);
            m_cubeMesh = mesh;
        }

        // Material uses sky shader
        m_material = std::make_shared<Material>("sky.material");
        m_registry.registerMaterial(m_material);
    }

    void SkyRenderer::render(IRenderBackend &backend, const Camera &cam) {
        if (!m_shader || !m_cubeMesh)
            return;

        backend.setDepthMask(false);
        // Compute inverse viewProjection
        glm::mat4 view = cam.GetViewMatrix();
        glm::mat4 proj = cam.GetProjectionMatrix();

        // Remove translation from view for skybox
        view[3] = glm::vec4(0, 0, 0, 1);

        glm::mat4 invVP = glm::inverse(proj * view);

        backend.setShader(m_shader);
        backend.setMaterial(*m_material);

        backend.setUniformMat4("uInvViewProj", invVP);
        backend.setUniformVec3("uSunDirection", m_sunDir);
        backend.setUniformVec3("uSunColor", m_sunColor);

        // Draw the cube
        backend.drawMesh(*m_cubeMesh, glm::mat4(1.0f));
        backend.setDepthMask(true);
    }

} // namespace Arche::Render
