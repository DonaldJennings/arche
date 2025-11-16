#pragma once
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Arche {
    namespace Render {

        class ResourceRegistry {
          public:
            // Mesh
            void registerMesh(std::shared_ptr<Mesh> mesh) {
                if (mesh)
                    m_meshes[mesh->getName()] = std::move(mesh);
            }
            std::shared_ptr<Mesh> getMesh(std::string_view name) const {
                auto it = m_meshes.find(std::string(name));
                return it != m_meshes.end() ? it->second : nullptr;
            }

            // Material
            void registerMaterial(std::shared_ptr<Material> mat) {
                if (mat)
                    m_materials[mat->getName()] = std::move(mat);
            }
            std::shared_ptr<Material> getMaterial(std::string_view name) const {
                auto it = m_materials.find(std::string(name));
                return it != m_materials.end() ? it->second : nullptr;
            }

            // Shader
            void registerShader(std::shared_ptr<Shader> sh) {
                if (sh)
                    m_shaders[sh->getName()] = std::move(sh);
            }
            std::shared_ptr<Shader> getShader(std::string_view name) const {
                auto it = m_shaders.find(std::string(name));
                return it != m_shaders.end() ? it->second : nullptr;
            }

            std::unordered_map<std::string, std::shared_ptr<Shader>> getAllShaders() const { return m_shaders; }

            std::unordered_map<std::string, std::shared_ptr<Material>> getAllMaterials() const { return m_materials; }

          private:
            std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;
            std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
            std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
        };

    } // namespace Render
} // namespace Arche