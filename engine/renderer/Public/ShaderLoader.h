#pragma once
#include <ResourceRegistry.h>
#include <Shader.h>
#include <filesystem>
#include <iostream>
#include <istream>
#include <memory>
#include <optional>
#include <string>

namespace Arche {
    namespace Render {

        struct TechniqueFile
        {
            std::string name;
            std::filesystem::path vertexPath;
            std::filesystem::path fragmentPath;

            std::filesystem::path geometryPath; // Optional
            std::filesystem::path computePath;  // Optional

            std::unordered_map<std::string, std::string> defines; // name -> type_string

            struct Property
            {
                std::string name;
                std::string type_string;
                std::string defaultValue;
            };

            std::vector<Property> properties;
        };

        class ShaderLoader {
          public:
            ShaderLoader(ResourceRegistry &registryIn, std::filesystem::path assetRoot)
                : registry{registryIn}, m_assetRoot{assetRoot} {}

            /**
             * @brief Function to load a shader from a file.
             * @param path The filesystem path to the shader file.
             * @return A shared pointer to the loaded Shader object, or nullptr if loading failed.
             */
            std::shared_ptr<Shader> load(const std::filesystem::path &path);

            void loadAllInDirectory()
            {
                // Iterate over all folders and in each folder call load on each .shader file
                for (const auto &entry : std::filesystem::recursive_directory_iterator(m_assetRoot)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".shader") {
                        load(entry.path());
                    }
                }
            }

          private:
            /**
             * @brief Read the contents of a file into a string.
             * @param path The filesystem path to the file.
             * @return A string containing the file contents.
             */
            std::string readFile(const std::filesystem::path &path);

            TechniqueFile parseTechniqueFile(const std::string &contents);

            ResourceRegistry& registry;

            std::filesystem::path m_assetRoot;
        };

    } // namespace Render
} // namespace Arche