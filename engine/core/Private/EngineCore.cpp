#include "EngineCore.h"

#include "EngineCore.h"
#include <memory> // Ensure this is included for std::shared_ptr

namespace
{
    std::shared_ptr<Arche::Render::Mesh> MakeUvSphereMesh(const std::string &name, float r, uint32_t segU,
                                                          uint32_t segV) {
        using Arche::Render::Mesh;
        auto mesh = std::make_shared<Mesh>(name);
        mesh->setPrimitive(Mesh::Primitive::Triangles);
        auto &verts = mesh->vertices();
        auto &inds = mesh->indices();
        verts.clear();
        inds.clear();

        for (uint32_t y = 0; y <= segV; ++y) {
            float v = float(y) / float(segV);
            float phi = v * glm::pi<float>(); // [0, pi]
            for (uint32_t x = 0; x <= segU; ++x) {
                float u = float(x) / float(segU);
                float theta = u * glm::two_pi<float>(); // [0, 2pi]

                glm::vec3 p(r * std::sin(phi) * std::cos(theta), r * std::cos(phi),
                            r * std::sin(phi) * std::sin(theta));
                glm::vec3 n = glm::normalize(p);
                glm::vec2 uv(u, 1.0f - v);

                Mesh::Vertex vert{};
                vert.position = p;
                vert.normal = n;
                vert.uv = uv;
                verts.push_back(vert);
            }
        }

        auto idx = [&](uint32_t x, uint32_t y) { return y * (segU + 1) + x; };
        for (uint32_t y = 0; y < segV; ++y) {
            for (uint32_t x = 0; x < segU; ++x) {
                uint32_t i0 = idx(x, y);
                uint32_t i1 = idx(x + 1, y);
                uint32_t i2 = idx(x, y + 1);
                uint32_t i3 = idx(x + 1, y + 1);
                // two triangles per quad
                inds.push_back(i0);
                inds.push_back(i2);
                inds.push_back(i1);
                inds.push_back(i1);
                inds.push_back(i2);
                inds.push_back(i3);
            }
        }
        return mesh;
    }

    std::shared_ptr<Arche::Render::Mesh> MakeCubeMesh(const std::string &name, float halfExtent) {
        using Arche::Render::Mesh;
        auto mesh = std::make_shared<Mesh>(name);
        mesh->setPrimitive(Mesh::Primitive::Triangles);
        auto &verts = mesh->vertices();
        auto &inds = mesh->indices();
        verts.clear();
        inds.clear();
        // Define 8 vertices of the cube
        glm::vec3 positions[8] = {
            {-halfExtent, -halfExtent, -halfExtent}, {halfExtent, -halfExtent, -halfExtent},
            {halfExtent, halfExtent, -halfExtent},   {-halfExtent, halfExtent, -halfExtent},
            {-halfExtent, -halfExtent, halfExtent},  {halfExtent, -halfExtent, halfExtent},
            {halfExtent, halfExtent, halfExtent},    {-halfExtent, halfExtent, halfExtent}};
        // Define cube faces (two triangles per face)
        uint32_t faceIndices[36] = {
            0, 1, 2, 2, 3, 0, // Back face
            4, 5, 6, 6, 7, 4, // Front face
            0, 4, 7, 7, 3, 0, // Left face
            1, 5, 6, 6, 2, 1, // Right face
            3, 2, 6, 6, 7, 3, // Top face
            0, 1, 5, 5, 4, 0  // Bottom face
        };
        for (const auto &pos : positions) {
            Mesh::Vertex vert{};
            vert.position = pos;
            // optional: supply normals/uvs later
            verts.push_back(vert);
        }
        inds.insert(inds.end(), std::begin(faceIndices), std::end(faceIndices));
        return mesh;
    }

    std::shared_ptr<Arche::Render::Mesh> MakePlaneMesh(const std::string &name, float sizeX, float sizeZ, uint32_t segX,
                                                  uint32_t segZ) {
        using Arche::Render::Mesh;
        auto mesh = std::make_shared<Mesh>(name);
        mesh->setPrimitive(Mesh::Primitive::Triangles);
        auto &verts = mesh->vertices();
        auto &inds = mesh->indices();
        verts.clear();
        inds.clear();
        for (uint32_t z = 0; z <= segZ; ++z) {
            float fz = float(z) / float(segZ);
            float posZ = (fz - 0.5f) * sizeZ;
            for (uint32_t x = 0; x <= segX; ++x) {
                float fx = float(x) / float(segX);
                float posX = (fx - 0.5f) * sizeX;
                glm::vec3 p(posX, 0.0f, posZ);
                glm::vec3 n(0.0f, 1.0f, 0.0f);
                glm::vec2 uv(fx, fz);
                Mesh::Vertex vert{};
                vert.position = p;
                vert.normal = n;
                vert.uv = uv;
                verts.push_back(vert);
            }
        }
        auto idx = [&](uint32_t x, uint32_t z) { return z * (segX + 1) + x; };
        for (uint32_t z = 0; z < segZ; ++z) {
            for (uint32_t x = 0; x < segX; ++x) {
                uint32_t i0 = idx(x, z);
                uint32_t i1 = idx(x + 1, z);
                uint32_t i2 = idx(x, z + 1);
                uint32_t i3 = idx(x + 1, z + 1);
                // two triangles per quad
                inds.push_back(i0);
                inds.push_back(i2);
                inds.push_back(i1);
                inds.push_back(i1);
                inds.push_back(i2);
                inds.push_back(i3);
            }
        }
        return mesh;
    }
}

namespace Arche {
    namespace Core {
        void EngineCore::registerSubsystem(std::shared_ptr<ISubsystem> newSubsystem) {
            subsystems.emplace_back(std::move(newSubsystem));
        }

        void EngineCore::initialise() {
            worldSystem->setPhysics(physicsSystem);
            renderingSystem->initialise();

            auto camera{std::make_shared<Render::Camera>()};
            camera->setPerspective(60.0, 16.0 / 9.0, 0.1, 1000.0);
            camera->SetPosition(glm::dvec3(0.0, 0.0, 10.0));
            renderingSystem->setMainCamera(camera);

            Render::ResourceRegistry &resources = renderingSystem->resources();

            // Simple unlit color shader (matches backend pipeline uniforms/attributes)
            static const char* kFlatVS = R"glsl(
#version 460 core
layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;
uniform float uPointSize;
void main(){
    gl_Position = uProj * uView * uModel * vec4(inPosition, 1.0);
    gl_PointSize = uPointSize; // used if drawing GL_POINTS
}
)glsl";
            static const char* kFlatFS = R"glsl(
#version 460 core
out vec4 fragColor;
uniform vec4 uBaseColor;
void main(){
    fragColor = uBaseColor;
}
)glsl";

            auto flatShader = std::make_shared<Render::Shader>(
                "flat.color",
                Render::Shader::Sources{std::string(kFlatVS), std::string(kFlatFS)});
            resources.registerShader(flatShader);

            {
                auto sphereMesh = std::make_shared<Render::Mesh>("sphere.mesh");
                sphereMesh->setPrimitive(Render::Mesh::Primitive::Points);
                Render::Mesh::Vertex vertex;
                vertex.position = glm::vec3(0.0f, 0.0f, 0.0f);
                sphereMesh->vertices().push_back(vertex);
                resources.registerMesh(sphereMesh);
            }
            {
                auto sphereMat = std::make_shared<Render::Material>("sphere.mat");
                sphereMat->setBaseColor(glm::vec4(0.9f, 0.6f, 0.2f, 1.0f));
                sphereMat->setPointSize(8.0f);
                sphereMat->setShader(flatShader); // attach simple color shader
                resources.registerMaterial(sphereMat);
            }
            {
                auto uvSphereMesh = MakeUvSphereMesh("uv_sphere.mesh", 1.0f, 16, 16);
                resources.registerMesh(uvSphereMesh);

                auto triMat = std::make_shared<Render::Material>("uv_sphere.mat");
                triMat->setBaseColor(glm::vec4(0.2f, 0.6f, 0.9f, 1.0f));
                triMat->setShader(flatShader); // same shader works for triangles
                resources.registerMaterial(triMat);
            }
            {
                auto cubeMesh = MakeCubeMesh("cube.mesh", 1.0f);
                resources.registerMesh(cubeMesh);

                auto cubeMat = std::make_shared<Render::Material>("cube.mat");
                cubeMat->setBaseColor(glm::vec4(0.2f, 0.9f, 0.3f, 1.0f));
                cubeMat->setShader(flatShader);
                resources.registerMaterial(cubeMat);
            }
            {
                auto planeMesh = MakePlaneMesh("plane.mesh", 10.0f, 10.0f, 10, 10);
                resources.registerMesh(planeMesh);
                auto planeMat = std::make_shared<Render::Material>("plane.mat");
                planeMat->setBaseColor(glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
                planeMat->setShader(flatShader);
                resources.registerMaterial(planeMat);
            }
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