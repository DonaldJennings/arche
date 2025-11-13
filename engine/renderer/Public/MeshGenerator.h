#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "Mesh.h"
#include "ResourceRegistry.h"

namespace Arche::Render {

    class MeshGenerator {
      public:
        // Registers all built-in procedural meshes (good for EngineCore startup)
        static void registerBuiltinMeshes(ResourceRegistry &registry);

        // Individual generators:
        static std::shared_ptr<Mesh> makePointSphere(const std::string &name);
        static std::shared_ptr<Mesh> makeUvSphere(const std::string &name, float radius, uint32_t segmentsU,
                                                  uint32_t segmentsV);
        static std::shared_ptr<Mesh> makeCube(const std::string &name, float size);
        static std::shared_ptr<Mesh> makePlane(const std::string &name, float width, float height, uint32_t segU,
                                               uint32_t segV);
    };

} // namespace Arche::Render
