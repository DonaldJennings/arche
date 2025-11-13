#include "MeshGenerator.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

namespace Arche::Render {

    // =======================================================
    // Register all default procedural meshes
    // =======================================================
    void MeshGenerator::registerBuiltinMeshes(ResourceRegistry &registry) {
        registry.registerMesh(makePointSphere("sphere.mesh"));
        registry.registerMesh(makeUvSphere("uv_sphere.mesh", 1.0f, 16, 16));
        registry.registerMesh(makeCube("cube.mesh", 1.0f));
        registry.registerMesh(makePlane("plane.mesh", 10.0f, 10.0f, 10, 10));
    }

    // =======================================================
    // Debug point sphere (GL_POINTS)
    // =======================================================
    std::shared_ptr<Mesh> MeshGenerator::makePointSphere(const std::string &name) {
        auto mesh = std::make_shared<Mesh>(name);
        mesh->setPrimitive(Mesh::Primitive::Points);

        Mesh::Vertex v;
        v.position = glm::vec3(0.0f, 0.0f, 0.0f);
        mesh->vertices().push_back(v);

        return mesh;
    }

    // =======================================================
    // UV Sphere
    // =======================================================
    std::shared_ptr<Mesh> MeshGenerator::makeUvSphere(const std::string &name, float radius, uint32_t segU,
                                                      uint32_t segV) {
        auto mesh = std::make_shared<Mesh>(name);
        mesh->setPrimitive(Mesh::Primitive::Triangles);

        auto &verts = mesh->vertices();
        auto &inds = mesh->indices();

        for (uint32_t y = 0; y <= segV; y++) {
            for (uint32_t x = 0; x <= segU; x++) {

                float u = (float)x / segU;
                float v = (float)y / segV;

                float theta = u * glm::two_pi<float>();
                float phi = v * glm::pi<float>();

                glm::vec3 pos(radius * std::sin(phi) * std::cos(theta), radius * std::cos(phi),
                              radius * std::sin(phi) * std::sin(theta));

                Mesh::Vertex vert;
                vert.position = pos;
                vert.normal = glm::normalize(pos);
                vert.uv = glm::vec2(u, 1.0f - v);

                verts.push_back(vert);
            }
        }

        // indices
        for (uint32_t y = 0; y < segV; y++) {
            for (uint32_t x = 0; x < segU; x++) {
                uint32_t i0 = y * (segU + 1) + x;
                uint32_t i1 = y * (segU + 1) + x + 1;
                uint32_t i2 = (y + 1) * (segU + 1) + x;
                uint32_t i3 = (y + 1) * (segU + 1) + x + 1;

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

    // =======================================================
    // Cube
    // =======================================================
    std::shared_ptr<Mesh> MeshGenerator::makeCube(const std::string &name, float size) {
        auto mesh = std::make_shared<Mesh>(name);
        mesh->setPrimitive(Mesh::Primitive::Triangles);

        float h = size * 0.5f;

        // 8 vertices
        glm::vec3 positions[] = {
            {-h, -h, -h}, {h, -h, -h}, {h, h, -h}, {-h, h, -h}, // back
            {-h, -h, h},  {h, -h, h},  {h, h, h},  {-h, h, h}   // front
        };

        uint32_t idx[] = {
            0, 1, 2, 0, 2, 3, // back
            4, 6, 5, 4, 7, 6, // front
            4, 5, 1, 4, 1, 0, // bottom
            3, 2, 6, 3, 6, 7, // top
            1, 5, 6, 1, 6, 2, // right
            4, 0, 3, 4, 3, 7  // left
        };

        auto &verts = mesh->vertices();
        auto &inds = mesh->indices();

        for (const auto &pos : positions) {
            Mesh::Vertex v{};
            v.position = pos;
            // simple normal = position normalized (placeholder)
            v.normal = glm::normalize(pos);
            verts.push_back(v);
        }

        for (uint32_t i : idx)
            inds.push_back(i);

        return mesh;
    }

    // =======================================================
    // Plane (X/Z grid)
    // =======================================================
    std::shared_ptr<Mesh> MeshGenerator::makePlane(const std::string &name, float width, float height, uint32_t segU,
                                                   uint32_t segV) {
        auto mesh = std::make_shared<Mesh>(name);
        mesh->setPrimitive(Mesh::Primitive::Triangles);

        auto &verts = mesh->vertices();
        auto &inds = mesh->indices();

        for (uint32_t y = 0; y <= segV; y++) {
            for (uint32_t x = 0; x <= segU; x++) {

                float u = (float)x / segU;
                float v = (float)y / segV;

                float px = (u - 0.5f) * width;
                float pz = (v - 0.5f) * height;

                Mesh::Vertex vert;
                vert.position = glm::vec3(px, 0.0f, pz);
                vert.normal = glm::vec3(0, 1, 0);
                vert.uv = glm::vec2(u, v);

                verts.push_back(vert);
            }
        }

        for (uint32_t y = 0; y < segV; y++) {
            for (uint32_t x = 0; x < segU; x++) {

                uint32_t i0 = y * (segU + 1) + x;
                uint32_t i1 = y * (segU + 1) + x + 1;
                uint32_t i2 = (y + 1) * (segU + 1) + x;
                uint32_t i3 = (y + 1) * (segU + 1) + x + 1;

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

} // namespace Arche::Render
