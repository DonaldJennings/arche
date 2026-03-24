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

        auto &verts = mesh->vertices();
        auto &inds  = mesh->indices();

        // 6 faces, 4 vertices each, 2 triangles each — 24 vertices total with flat normals.
        // Winding is CCW when viewed from outside (front face).
        struct FaceDesc {
            glm::vec3 normal;
            glm::vec3 p[4]; // CCW from outside
        };

        FaceDesc faces[] = {
            // +Z front
            { {0,0,1}, { {-h,-h,h},{h,-h,h},{h,h,h},{-h,h,h} } },
            // -Z back
            { {0,0,-1}, { {h,-h,-h},{-h,-h,-h},{-h,h,-h},{h,h,-h} } },
            // +X right
            { {1,0,0}, { {h,-h,h},{h,-h,-h},{h,h,-h},{h,h,h} } },
            // -X left
            { {-1,0,0}, { {-h,-h,-h},{-h,-h,h},{-h,h,h},{-h,h,-h} } },
            // +Y top
            { {0,1,0}, { {-h,h,h},{h,h,h},{h,h,-h},{-h,h,-h} } },
            // -Y bottom
            { {0,-1,0}, { {-h,-h,-h},{h,-h,-h},{h,-h,h},{-h,-h,h} } },
        };

        for (const auto &face : faces) {
            uint32_t base = static_cast<uint32_t>(verts.size());

            glm::vec2 uvs[4] = { {0,0},{1,0},{1,1},{0,1} };
            for (int i = 0; i < 4; ++i) {
                Mesh::Vertex v{};
                v.position = face.p[i];
                v.normal   = face.normal;
                v.uv       = uvs[i];
                verts.push_back(v);
            }

            // Two CCW triangles: 0,1,2 and 0,2,3
            inds.push_back(base + 0);
            inds.push_back(base + 1);
            inds.push_back(base + 2);
            inds.push_back(base + 0);
            inds.push_back(base + 2);
            inds.push_back(base + 3);
        }

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
