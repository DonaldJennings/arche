#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Arche {
    namespace Render {

        // Renderer-agnostic mesh data. Backend uploads it to GPU and caches VAO/VBO/IBO.
        class Mesh {
          public:
            enum class Primitive { Points, Lines, Triangles };

            struct Vertex {
                glm::vec3 position{};
                glm::vec3 normal{};
                glm::vec2 uv{};
            };

            Mesh() = default;
            explicit Mesh(std::string name) : m_name(std::move(name)) {}

            const std::string& getName() const { return m_name; }
            void setName(std::string n) { m_name = std::move(n); }

            void setPrimitive(Primitive p) { m_primitive = p; }
            Primitive getPrimitive() const { return m_primitive; }

            std::vector<Vertex>& vertices() { return m_vertices; }
            const std::vector<Vertex>& vertices() const { return m_vertices; }

            std::vector<std::uint32_t>& indices() { return m_indices; }
            const std::vector<std::uint32_t>& indices() const { return m_indices; }

            bool hasIndices() const { return !m_indices.empty(); }

            // Optional helpers
            void clear() { m_vertices.clear(); m_indices.clear(); }
            size_t vertexCount() const { return m_vertices.size(); }
            size_t indexCount() const { return m_indices.size(); }

          private:
            std::string m_name;
            Primitive m_primitive{Primitive::Triangles};
            std::vector<Vertex> m_vertices;
            std::vector<std::uint32_t> m_indices;
        };

    } // namespace Render
} // namespace Arche