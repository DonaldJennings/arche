#pragma once
#include "ShaderLoader.h"
#include <ResourceRegistry.h>
#include <Shader.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

namespace Arche {
    namespace Render {

        std::shared_ptr<Shader> ShaderLoader::load(const std::filesystem::path &path) {

            std::filesystem::path absolutePath = m_assetRoot / path;

            std::string contents{readFile(absolutePath)};

            TechniqueFile techniqueFile{parseTechniqueFile(contents)};

            std::string vertexSrc{readFile(absolutePath.parent_path() / techniqueFile.vertexPath)};
            std::string fragmentSrc{readFile(absolutePath.parent_path() / techniqueFile.fragmentPath)};

            Shader::Sources sources;
            sources.vertexGLSL = vertexSrc;
            sources.fragmentGLSL = fragmentSrc;

            auto shader = std::make_shared<Shader>(techniqueFile.name, sources);

            for (const auto &define : techniqueFile.properties) {
                shader->setDefine(define.name, define.type_string);
            }

            registry.registerShader(shader);

            return shader;
        }

        std::string ShaderLoader::readFile(const std::filesystem::path &path) {
            std::ifstream file(path, std::ios::in | std::ios::binary);

            if (!file.is_open()) {
                throw std::runtime_error("ShaderLoader: Failed to open file: " + path.string());
            }

            // Seek to end and reserve
            file.seekg(0, std::ios::end);
            std::string content;
            content.resize(file.tellg());
            file.seekg(0, std::ios::beg);

            // Read everything
            file.read(&content[0], content.size());

            if (!file) {
                throw std::runtime_error("ShaderLoader: Failed to read file: " + path.string());
            }

            return content;
        }

        TechniqueFile ShaderLoader::parseTechniqueFile(const std::string &contents) {

            TechniqueFile results;

            enum class Section { None, Defines, Properties };

            Section current = Section::None;
            std::istringstream stream{contents};
            std::string line;

            auto trim = [](std::string s) {
                auto notSpace = [](char c) { return !std::isspace(static_cast<unsigned char>(c)); };
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
                s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
                return s;
            };

            while (std::getline(stream, line)) {
                line = trim(line);
                if (line.empty() || line[0] == '#')
                    continue;

                // Section switching
                if (line == "[Defines]") {
                    current = Section::Defines;
                    continue;
                } else if (line == "[Properties]") {
                    current = Section::Properties;
                    continue;
                }

                // -------------------------------------------
                // SECTION: NONE  (technique / vertex / fragment)
                // -------------------------------------------
                if (current == Section::None) {

                    // --- technique ---
                    if (line.rfind("technique", 0) == 0) {
                        auto pos = line.find_first_of(" =");
                        if (pos == std::string::npos)
                            throw std::runtime_error("Invalid technique declaration");

                        results.name = trim(line.substr(pos + 1));
                        continue;
                    }

                    // Helper that extracts value after '=' or space
                    auto parseKeyValue = [&](const std::string &src) -> std::string {
                        // try "keyword = value"
                        auto eq = src.find('=');
                        if (eq != std::string::npos)
                            return trim(src.substr(eq + 1));

                        // try "keyword value"
                        auto sp = src.find(' ');
                        if (sp != std::string::npos)
                            return trim(src.substr(sp + 1));

                        throw std::runtime_error("Malformed line: " + src);
                    };

                    // --- vertex ---
                    if (line.rfind("vertex", 0) == 0) {
                        results.vertexPath = parseKeyValue(line);
                        continue;
                    }

                    // --- fragment ---
                    if (line.rfind("fragment", 0) == 0) {
                        results.fragmentPath = parseKeyValue(line);
                        continue;
                    }

                    continue; // nothing else in Section::None
                }

                // -------------------------------------------
                // SECTION: DEFINES
                // -------------------------------------------
                if (current == Section::Defines) {
                    auto eq = line.find('=');
                    if (eq == std::string::npos)
                        throw std::runtime_error("Invalid define: expected NAME = VALUE");

                    std::string name = trim(line.substr(0, eq));
                    std::string value = trim(line.substr(eq + 1));

                    if (name.empty() || value.empty())
                        throw std::runtime_error("Invalid define: empty key or value");

                    results.defines[name] = value;
                    continue;
                }

                // -------------------------------------------
                // SECTION: PROPERTIES
                // -------------------------------------------
                if (current == Section::Properties) {
                    auto colon = line.find(':');
                    if (colon == std::string::npos)
                        throw std::runtime_error("Malformed property: expected 'name : type = default'");

                    std::string propName = trim(line.substr(0, colon));
                    std::string rest = trim(line.substr(colon + 1));

                    auto eq = rest.find('=');
                    if (eq == std::string::npos)
                        throw std::runtime_error("Malformed property: missing '='");

                    std::string type = trim(rest.substr(0, eq));
                    std::string defaultVal = trim(rest.substr(eq + 1));

                    results.properties.push_back({propName, type, defaultVal});
                    continue;
                }
            }

            // Validation
            if (results.name.empty())
                throw std::runtime_error("Technique file missing 'technique' name.");
            if (results.vertexPath.empty())
                throw std::runtime_error("Technique missing 'vertex' stage.");
            if (results.fragmentPath.empty())
                throw std::runtime_error("Technique missing 'fragment' stage.");

            return results;
        }

    } // namespace Render
} // namespace Arche