#pragma once
#include <memory>
#include <string>
#include <unordered_map>

namespace Arche {
    namespace Render {

        /**
         * @brief Platform-agnostic shader program representation.
         * 
         * Shader holds the source code for vertex and fragment shaders,
         * along with optional preprocessor defines. The rendering backend
         * is responsible for compiling and caching the actual GPU programs.
         * 
         * Shaders are loaded from files by the ShaderLoader and stored in
         * the ResourceRegistry for efficient reuse.
         */
        class Shader {
          public:
            /**
             * @brief Container for shader source code.
             */
            struct Sources {
                std::string vertexGLSL;    ///< Vertex shader GLSL source
                std::string fragmentGLSL;  ///< Fragment shader GLSL source
            };

            /**
             * @brief Default constructor for empty shader.
             */
            Shader() = default;
            
            /**
             * @brief Construct a shader with name and sources.
             * 
             * @param name Shader identifier
             * @param sources Vertex and fragment shader source code
             */
            Shader(std::string name, Sources sources) : m_name(std::move(name)), m_sources(std::move(sources)) {}

            /**
             * @brief Get the shader's name.
             * @return Shader identifier
             */
            const std::string &getName() const { return m_name; }
            
            /**
             * @brief Get the shader source code.
             * @return Sources structure with vertex and fragment code
             */
            const Sources &getSources() const { return m_sources; }

            /**
             * @brief Set a preprocessor define for shader compilation.
             * 
             * Defines can be used to create shader variants (e.g., with/without
             * textures) without duplicating source files.
             * 
             * @param key Define name (e.g., "USE_TEXTURE")
             * @param value Define value (e.g., "1")
             */
            void setDefine(std::string key, std::string value) { m_defines.emplace(std::move(key), std::move(value)); }
            
            /**
             * @brief Get all preprocessor defines.
             * @return Map of define names to values
             */
            const std::unordered_map<std::string, std::string> &getDefines() const { return m_defines; }

          private:
            std::string m_name;                                         ///< Shader identifier
            Sources m_sources;                                          ///< Source code
            std::unordered_map<std::string, std::string> m_defines;    ///< Preprocessor defines
        };

    } // namespace Render
} // namespace Arche