#pragma once
#include <memory>
#include <string>
#include <string_view>

namespace Arche {
    namespace Render {

        class IRenderable {
          public:
            virtual ~IRenderable() = default;
            virtual std::string_view getMeshName() const = 0;
            virtual std::string_view getMaterialName() const = 0;
        };


        class NamedRenderable : public IRenderable {
          public:
            NamedRenderable(std::string meshName, std::string materialName)
                : m_meshName(std::move(meshName)), m_materialName(std::move(materialName)) {}
            std::string_view getMeshName() const override { return m_meshName; }
            std::string_view getMaterialName() const override { return m_materialName; }
            void setMaterialName(std::string materialName) { m_materialName = std::move(materialName); }
            void setMeshName(std::string meshName) { m_meshName = std::move(meshName); }
          private:
            std::string m_meshName;
            std::string m_materialName;
        };

    } // namespace Scene
} // namespace Arche