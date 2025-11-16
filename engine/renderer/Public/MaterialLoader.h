#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "Material.h"
#include "ResourceRegistry.h"

namespace Arche::Render {

    /**
     * @brief Utility for loading material files from disk.
     * 
     * MaterialLoader reads .mat files (custom material format) from the
     * asset directory and creates Material objects that are registered
     * with the ResourceRegistry for use throughout the engine.
     * 
     * Material files are simple text-based format defining shader references,
     * colors, textures, and custom parameters.
     */
    class MaterialLoader {
      public:
        /**
         * @brief Construct a material loader.
         * 
         * @param registry Resource registry to populate with loaded materials
         * @param assetRoot Root directory containing material files
         */
        MaterialLoader(ResourceRegistry &registry, std::filesystem::path assetRoot);

        /**
         * @brief Load a single material file.
         * 
         * Reads and parses a .mat file, creates a Material object, and
         * registers it with the resource registry.
         * 
         * @param relativePath Path to .mat file relative to asset root
         * @return Shared pointer to the loaded material, or nullptr on error
         */
        std::shared_ptr<Material> load(const std::filesystem::path &relativePath);
        
        /**
         * @brief Load all materials in the asset directory.
         * 
         * Recursively searches the asset root for .mat files and loads
         * all of them. Useful for batch loading during initialization.
         */
        void loadAllInDirectory() {
            // Iterate over all folders and in each folder call load on each .shader file
            for (const auto &entry : std::filesystem::recursive_directory_iterator(m_assetRoot)) {
                if (entry.is_regular_file() && entry.path().extension() == ".mat") {
                    load(entry.path());
                }
            }
        }

      private:
        ResourceRegistry &m_registry;           ///< Registry to populate
        std::filesystem::path m_assetRoot;     ///< Asset directory root

        /**
         * @brief Read entire file into a string.
         * @param path File path to read
         * @return File contents as string
         */
        std::string readFile(const std::filesystem::path &path);

        /**
         * @brief Parsed material file data.
         */
        struct ParsedMaterial {
            std::unordered_map<std::string, std::string> values;  ///< Key-value pairs from file
        };

        /**
         * @brief Parse material file format.
         * @param text File contents
         * @return Parsed material data
         */
        ParsedMaterial parseMaterialFile(const std::string &text);
    };

} // namespace Arche::Render
