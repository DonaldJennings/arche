    #pragma once
#include <memory>
#include <string>
#include <unordered_map>

namespace Arche {
    namespace Render {

        // Renderer-agnostic shader description (no GL includes here).
        // Backend compiles and caches GPU programs from these sources.
        class Shader {
          public:
            struct Sources {
                std::string vertexGLSL;
                std::string fragmentGLSL;
            };

            Shader() = default;
            Shader(std::string name, Sources sources)
                : m_name(std::move(name)), m_sources(std::move(sources)) {}

            const std::string& getName() const { return m_name; }
            const Sources& getSources() const { return m_sources; }

            // Optional defines (name->value) usable by backend before compile
            void setDefine(std::string key, std::string value) { m_defines.emplace(std::move(key), std::move(value)); }
            const std::unordered_map<std::string, std::string>& getDefines() const { return m_defines; }

          private:
            std::string m_name;
            Sources m_sources;
            std::unordered_map<std::string, std::string> m_defines;
        };

    } // namespace Render
} // namespace Arche