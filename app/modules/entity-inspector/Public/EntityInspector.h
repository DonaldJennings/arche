#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "EditorSession.h"
#include <IPanel.h>
#include <WorldSystem.h>

namespace Arche {
    namespace Render {
        class Material;
        class ResourceRegistry;
    }
}

namespace Arche {
    namespace GUI {

        class EntityInspector : public IPanel {
          public:
            explicit EntityInspector(std::shared_ptr<EditorSession> contextIn)
                : context{contextIn}, name{"Entity Inspector"} {}

            void Draw() override;
            std::string_view GetName() const override;

          private:
            std::string name;
            std::shared_ptr<EditorSession> context;

            std::shared_ptr<Scene::IEntity> resolveSelectedEntity() const;
            static std::string makeEntityMaterialPrefix(std::uint64_t entityId);
            static std::string makeEntityMaterialName(std::uint64_t entityId, std::string_view baseName);
            static bool isEntityMaterialOverride(std::uint64_t entityId, std::string_view materialName);
            std::shared_ptr<Render::Material>
            ensureEntityMaterialOverride(const std::shared_ptr<Scene::IEntity> &entity,
                                         Render::ResourceRegistry &registry,
                                         std::shared_ptr<Render::Material> currentMaterial) const;
        };

    } // namespace GUI
} // namespace Arche