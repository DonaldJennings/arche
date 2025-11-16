#pragma once
#include <memory>
#include <string>
#include <string_view>

namespace Arche {
    namespace Render {

        /**
         * @brief Interface for renderable objects.
         * 
         * IRenderable defines the contract for objects that can be drawn by
         * the rendering system. It provides access to the mesh and material
         * resource names needed for rendering.
         */
        class IRenderable {
          public:
            /**
             * @brief Virtual destructor for proper cleanup.
             */
            virtual ~IRenderable() = default;
            
            /**
             * @brief Get the mesh resource name.
             * @return String view of the mesh identifier
             */
            virtual std::string_view getMeshName() const = 0;
            
            /**
             * @brief Get the material resource name.
             * @return String view of the material identifier
             */
            virtual std::string_view getMaterialName() const = 0;
        };


        /**
         * @brief Simple implementation of IRenderable using string names.
         * 
         * NamedRenderable stores mesh and material resource names as strings
         * and provides mutable access to them. This is the most common
         * implementation used by entities.
         */
        class NamedRenderable : public IRenderable {
          public:
            /**
             * @brief Construct a renderable with mesh and material names.
             * 
             * @param meshName Name of the mesh resource to render
             * @param materialName Name of the material resource to use
             */
            NamedRenderable(std::string meshName, std::string materialName)
                : m_meshName(std::move(meshName)), m_materialName(std::move(materialName)) {}
            
            std::string_view getMeshName() const override { return m_meshName; }
            std::string_view getMaterialName() const override { return m_materialName; }
            
            /**
             * @brief Change the material resource.
             * @param materialName New material name
             */
            void setMaterialName(std::string materialName) { m_materialName = std::move(materialName); }
            
            /**
             * @brief Change the mesh resource.
             * @param meshName New mesh name
             */
            void setMeshName(std::string meshName) { m_meshName = std::move(meshName); }
            
          private:
            std::string m_meshName;      ///< Mesh resource identifier
            std::string m_materialName;  ///< Material resource identifier
        };

    } // namespace Scene
} // namespace Arche