#pragma once
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Arche {
    namespace Render {

        /**
         * @brief Central registry for rendering resources.
         * 
         * ResourceRegistry provides a name-based lookup system for all
         * rendering resources (meshes, materials, shaders). Resources are
         * registered by loaders during initialization and retrieved by
         * render passes and systems during rendering.
         * 
         * This acts as the single source of truth for all loaded graphics
         * assets, preventing duplicate loading and ensuring consistent access.
         */
        class ResourceRegistry {
          public:
            /**
             * @brief Register a mesh resource.
             * 
             * Adds a mesh to the registry under its name. If a mesh with
             * the same name already exists, it is replaced.
             * 
             * @param mesh Shared pointer to the mesh to register
             */
            void registerMesh(std::shared_ptr<Mesh> mesh) {
                if (mesh)
                    m_meshes[mesh->getName()] = std::move(mesh);
            }
            
            /**
             * @brief Retrieve a mesh by name.
             * 
             * @param name Mesh identifier
             * @return Shared pointer to the mesh, or nullptr if not found
             */
            std::shared_ptr<Mesh> getMesh(std::string_view name) const {
                auto it = m_meshes.find(std::string(name));
                return it != m_meshes.end() ? it->second : nullptr;
            }

            /**
             * @brief Register a material resource.
             * 
             * Adds a material to the registry under its name. If a material
             * with the same name already exists, it is replaced.
             * 
             * @param mat Shared pointer to the material to register
             */
            void registerMaterial(std::shared_ptr<Material> mat) {
                if (mat)
                    m_materials[mat->getName()] = std::move(mat);
            }
            
            /**
             * @brief Retrieve a material by name.
             * 
             * @param name Material identifier
             * @return Shared pointer to the material, or nullptr if not found
             */
            std::shared_ptr<Material> getMaterial(std::string_view name) const {
                auto it = m_materials.find(std::string(name));
                return it != m_materials.end() ? it->second : nullptr;
            }

            /**
             * @brief Register a shader resource.
             * 
             * Adds a shader to the registry under its name. If a shader
             * with the same name already exists, it is replaced.
             * 
             * @param sh Shared pointer to the shader to register
             */
            void registerShader(std::shared_ptr<Shader> sh) {
                if (sh)
                    m_shaders[sh->getName()] = std::move(sh);
            }
            
            /**
             * @brief Retrieve a shader by name.
             * 
             * @param name Shader identifier
             * @return Shared pointer to the shader, or nullptr if not found
             */
            std::shared_ptr<Shader> getShader(std::string_view name) const {
                auto it = m_shaders.find(std::string(name));
                return it != m_shaders.end() ? it->second : nullptr;
            }

            /**
             * @brief Get all registered shaders.
             * 
             * @return Copy of the shader map
             */
            std::unordered_map<std::string, std::shared_ptr<Shader>> getAllShaders() const { return m_shaders; }

            /**
             * @brief Get all registered meshes.
             *
             * @return Copy of the mesh map
             */
            std::unordered_map<std::string, std::shared_ptr<Mesh>> getAllMeshes() const { return m_meshes; }

            /**
             * @brief Get all registered materials.
             *
             * @return Copy of the material map
             */
            std::unordered_map<std::string, std::shared_ptr<Material>> getAllMaterials() const { return m_materials; }

          private:
            std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;          ///< Mesh registry
            std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;   ///< Material registry
            std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;       ///< Shader registry
        };

    } // namespace Render
} // namespace Arche