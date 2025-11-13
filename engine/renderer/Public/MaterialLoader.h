#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "Material.h"
#include "ResourceRegistry.h"

namespace Arche::Render {

    class MaterialLoader {
      public:
        MaterialLoader(ResourceRegistry &registry, std::filesystem::path assetRoot);

        // Load a material relative to the asset root
        std::shared_ptr<Material> load(const std::filesystem::path &relativePath);

      private:
        ResourceRegistry &m_registry;
        std::filesystem::path m_assetRoot;

        std::string readFile(const std::filesystem::path &path);

        struct ParsedMaterial {
            std::unordered_map<std::string, std::string> values;
        };

        ParsedMaterial parseMaterialFile(const std::string &text);
    };

} // namespace Arche::Render
