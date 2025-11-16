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

        /**
         * @brief Shader technique file metadata.
         * 
         * Represents the parsed contents of a .shader file, which describes
         * how to build a shader program from GLSL source files and preprocessor
         * defines.
         */
        struct TechniqueFile
        {
            std::string name;                       ///< Shader technique name
            std::filesystem::path vertexPath;       ///< Vertex shader source file
            std::filesystem::path fragmentPath;     ///< Fragment shader source file

            std::filesystem::path geometryPath;     ///< Optional geometry shader
            std::filesystem::path computePath;      ///< Optional compute shader

            std::unordered_map<std::string, std::string> defines; ///< Preprocessor defines

            /**
             * @brief Shader property/uniform definition.
             */
            struct Property
            {
                std::string name;           ///< Property name
                std::string type_string;    ///< Property type
                std::string defaultValue;   ///< Default value
            };

            std::vector<Property> properties;       ///< Exposed shader properties
        };

        /**
         * @brief Utility for loading shader files from disk.
         * 
         * ShaderLoader reads .shader files (custom shader metadata format)
         * from the asset directory, loads the referenced GLSL source files,
         * and creates Shader objects that are registered with the
         * ResourceRegistry for use throughout the engine.
         * 
         * Shader files are text-based metadata that reference actual GLSL
         * source files and define preprocessor symbols and properties.
         */
        class ShaderLoader {
          public:
            /**
             * @brief Construct a shader loader.
             * 
             * @param registryIn Resource registry to populate with loaded shaders
             * @param assetRoot Root directory containing shader files
             */
            ShaderLoader(ResourceRegistry &registryIn, std::filesystem::path assetRoot)
                : registry{registryIn}, m_assetRoot{assetRoot} {}

            /**
             * @brief Load a single shader from a .shader file.
             * 
             * Reads and parses a .shader metadata file, loads the referenced
             * GLSL source files, creates a Shader object, and registers it
             * with the resource registry.
             * 
             * @param path Filesystem path to the .shader file
             * @return Shared pointer to the loaded Shader object, or nullptr if loading failed
             */
            std::shared_ptr<Shader> load(const std::filesystem::path &path);

            /**
             * @brief Load all shaders in the asset directory.
             * 
             * Recursively searches the asset root for .shader files and loads
             * all of them. Useful for batch loading during initialization.
             */
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
             * @brief Read entire file into a string.
             * @param path File path to read
             * @return File contents as string
             */
            std::string readFile(const std::filesystem::path &path);

            /**
             * @brief Parse a .shader technique file.
             * 
             * @param contents File contents
             * @return Parsed technique metadata
             */
            TechniqueFile parseTechniqueFile(const std::string &contents);

            ResourceRegistry& registry;             ///< Registry to populate

            std::filesystem::path m_assetRoot;     ///< Asset directory root
        };

    } // namespace Render
} // namespace Arche