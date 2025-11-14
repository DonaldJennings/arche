#include "SkyPass.h"
#include <glm/gtc/matrix_inverse.hpp>

namespace Arche::Render {
    void SkyPass::initialise(IRenderBackend &backend) {}

    void SkyPass::render(const RenderView &view, IRenderBackend &backend, const Camera &camera,
                         ResourceRegistry &resources, const Core::RenderSettings &settings) {
        // Create a cube mesh if not already present
        auto cubeMesh = resources.getMesh("skybox.cube");
        if (!cubeMesh) {
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

            resources.registerMesh(mesh);
            cubeMesh = mesh;
        }

        // Material uses sky shader
        auto material = std::make_shared<Material>("sky.material");
        resources.registerMaterial(material);

        backend.setDepthMask(false);

        // Remove translation from view for skybox

        glm::mat4 adjustedViewMatrix = view.viewMatrix;
        adjustedViewMatrix[3] = glm::vec4(0, 0, 0, 1);

        glm::mat4 invVP = glm::inverse(view.projectionMatrix * adjustedViewMatrix);

        if (std::shared_ptr<Shader> skyShader = resources.getShader("Sky")) {
            backend.setShader(skyShader);

            backend.setMaterial(*material);

            uploaderUniformProperties(backend, invVP);

            // Draw the cube
            backend.drawMesh(*cubeMesh, glm::mat4(1.0f));
            backend.setDepthMask(true);
        }
    }

    void SkyPass::shutdown(IRenderBackend &backend) 
    {
        // No resources to release for sky pass currently.
        // If you add GPU resources (textures, buffers), release them here.
    }

    void SkyPass::uploaderUniformProperties(IRenderBackend &backend, glm::mat4 inverseViewProjection) {
        backend.setUniformMat4("uInvViewProj", inverseViewProjection);
        backend.setUniformVec3("uSunDirection", m_sunDirection);
        backend.setUniformVec3("uSunColor", m_sunColor);
    };

} // namespace Arche::Render
