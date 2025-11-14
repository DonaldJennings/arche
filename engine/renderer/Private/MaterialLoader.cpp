#include "MaterialLoader.h"
#include <algorithm>
#include <fstream>
#include <glm/glm.hpp>
#include <sstream>
#include <stdexcept>
#include <LoggingService.h>

namespace Arche::Render {

    // ------------------------------------------------------------
    // Constructor
    // ------------------------------------------------------------
    MaterialLoader::MaterialLoader(ResourceRegistry &registry, std::filesystem::path assetRoot)
        : m_registry(registry), m_assetRoot(std::move(assetRoot)) {}

    // ------------------------------------------------------------
    // File reading helper
    // ------------------------------------------------------------
    std::string MaterialLoader::readFile(const std::filesystem::path &path) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("MaterialLoader: Unable to open file: " + path.string());

    fileseek:
        file.seekg(0, std::ios::end);
        std::string data;
        data.resize(file.tellg());

        file.seekg(0, std::ios::beg);
        file.read(&data[0], data.size());

        return data;
    }

    // ------------------------------------------------------------
    // Parse .mat text file into key/value map
    // ------------------------------------------------------------
    MaterialLoader::ParsedMaterial MaterialLoader::parseMaterialFile(const std::string &text) {
        ParsedMaterial result;

        auto trim = [](std::string s) {
            auto ns = [](unsigned char c) { return !std::isspace(c); };
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), ns));
            s.erase(std::find_if(s.rbegin(), s.rend(), ns).base(), s.end());
            return s;
        };

        std::istringstream ss(text);
        std::string line;

        while (std::getline(ss, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#')
                continue;

            auto eq = line.find('=');
            if (eq == std::string::npos)
                throw std::runtime_error("Invalid .mat line: " + line);

            std::string key = trim(line.substr(0, eq));
            std::string value = trim(line.substr(eq + 1));

            result.values[key] = value;
        }

        return result;
    }

    // ------------------------------------------------------------
    // Load material from disk and register it
    // ------------------------------------------------------------
    std::shared_ptr<Material> MaterialLoader::load(const std::filesystem::path &relativePath) {
        // Absolute path to .mat file
        auto fullPath = m_assetRoot / relativePath;

        // Read material text
        std::string text = readFile(fullPath);

        // Parse key/value pairs (unchanged)
        ParsedMaterial parsed = parseMaterialFile(text);

        // Material name from filename
        std::string matName = relativePath.filename().string();

        // Create material object
        auto material = std::make_shared<Material>(matName);

        // ------------------------------------------------------------
        // 1. Load SHADER FIRST if provided
        // ------------------------------------------------------------
        if (auto it = parsed.values.find("shader"); it != parsed.values.end()) {
            std::string shaderName = it->second;

            // Attempt to get shader from registry
            auto shader = m_registry.getShader(shaderName);

            if (!shader) {
                std::cerr << "MaterialLoader: Warning: Shader '" << shaderName << "' not found for material '"
                          << matName << "'. Using no shader." << std::endl;
            } else {
                material->setShaderName(shaderName);
            }

            // Remove shader from parsed list so it isn't treated as a uniform
            parsed.values.erase(it);
        } else {
            std::cerr << "MaterialLoader: Warning: No shader specified for material '" << matName
                      << "'. Using default flat shader." << std::endl;
            material->setShaderName("flat");
        }

        // ------------------------------------------------------------
        // 2. Parse property keys (vec4, vec3, float, texture)
        // ------------------------------------------------------------
        for (auto &[key, value] : parsed.values) {
            auto toLower = [](std::string s) {
                std::transform(s.begin(), s.end(), s.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return s;
            };

            std::string k = toLower(key);

            // ------------------------------------------------------------
            // Vec3 / Vec4 detection
            // ------------------------------------------------------------
            size_t c1 = value.find(',');
            if (c1 != std::string::npos) {
                size_t c2 = value.find(',', c1 + 1);
                size_t c3 = (c2 != std::string::npos) ? value.find(',', c2 + 1) : std::string::npos;

                // vec4
                if (c3 != std::string::npos) {
                    float r = std::stof(value.substr(0, c1));
                    float g = std::stof(value.substr(c1 + 1, c2 - c1 - 1));
                    float b = std::stof(value.substr(c2 + 1, c3 - c2 - 1));
                    float a = std::stof(value.substr(c3 + 1));

                    material->setVec4(key, glm::vec4(r, g, b, a));
                    continue;
                }

                // vec3
                if (c2 != std::string::npos) {
                    float r = std::stof(value.substr(0, c1));
                    float g = std::stof(value.substr(c1 + 1, c2 - c1 - 1));
                    float b = std::stof(value.substr(c2 + 1));

                    material->setVec3(key, glm::vec3(r, g, b));
                    continue;
                }
            }

            // ------------------------------------------------------------
            // Float
            // ------------------------------------------------------------
            try {
                float f = std::stof(value);
                material->setFloat(key, f);
                continue;
            } catch (...) {
                // not a float
            }

            // ------------------------------------------------------------
            // Texture path or string fallback
            // ------------------------------------------------------------
            material->setTexture(key, value);
        }

        // ------------------------------------------------------------
        // 3. Register the material
        // ------------------------------------------------------------
        m_registry.registerMaterial(material);
        return material;
    }

} // namespace Arche::Render
