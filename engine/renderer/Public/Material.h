#pragma once
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Shader.h"

namespace Arche {
    namespace Render {

        /**
         * @brief Platform-agnostic material properties container.
         * 
         * Material encapsulates all visual properties for rendering an object,
         * including colors, texture references, and custom shader parameters.
         * Materials reference shaders by name and can be configured with
         * arbitrary float/vec3/vec4 parameters for flexibility.
         * 
         * Texture references are symbolic (string names) - the backend resolves
         * them to actual GPU textures. This allows materials to be defined before
         * textures are loaded.
         * 
         * Materials are typically loaded from .mat files by MaterialLoader and
         * stored in the ResourceRegistry.
         */
        class Material {
          public:
            /**
             * @brief Default constructor for empty material.
             */
            Material() = default;
            
            /**
             * @brief Construct a named material.
             * @param name Material identifier
             */
            explicit Material(std::string name) : m_name(std::move(name)) {}

            /**
             * @brief Get the material's name.
             * @return Material identifier
             */
            const std::string& getName() const { return m_name; }
            
            /**
             * @brief Set the material's name.
             * @param name New material identifier
             */
            void setName(std::string name) { m_name = std::move(name); }
            
            /**
             * @brief Set the base/albedo color.
             * @param c RGBA color (alpha used for transparency)
             */
            void setBaseColor(const glm::vec4& c) { m_baseColor = c; }
            
            /**
             * @brief Get the base/albedo color.
             * @return RGBA color vector
             */
            const glm::vec4& getBaseColor() const { return m_baseColor; }

            /**
             * @brief Set point size for point rendering.
             * @param s Point size in pixels
             */
            void setPointSize(float s) { m_pointSize = s; }
            
            /**
             * @brief Get point size.
             * @return Point size in pixels
             */
            float getPointSize() const { return m_pointSize; }

            /**
             * @brief Assign a texture to a named slot.
             * 
             * Texture slots are symbolic names like "albedo", "normal", "roughness".
             * The shader is responsible for sampling the correct slots.
             * 
             * @param slot Texture slot name
             * @param textureName Texture resource identifier
             */
            void setTexture(std::string slot, std::string textureName) { m_textures[std::move(slot)] = std::move(textureName); }
            
            /**
             * @brief Get all texture assignments.
             * @return Map of slot names to texture names
             */
            const std::unordered_map<std::string, std::string>& getTextures() const { return m_textures; }

            /**
             * @brief Set the shader to use with this material.
             * @param shaderName Shader resource identifier
             */
            void setShaderName(std::string shaderName) { m_shaderName = std::move(shaderName); }
            
            /**
             * @brief Get the assigned shader name.
             * @return Shader identifier
             */
            const std::string &getShaderName() const { return m_shaderName; }

            /**
             * @brief Set a custom float parameter.
             * @param name Parameter name (must match shader uniform)
             * @param v Float value
             */
            void setFloat(std::string name, float v) { m_floats[std::move(name)] = v; }
            
            /**
             * @brief Get all float parameters.
             * @return Map of parameter names to float values
             */
            const std::unordered_map<std::string, float>& getFloats() const { return m_floats; }

            /**
             * @brief Set a custom vec3 parameter.
             * @param name Parameter name (must match shader uniform)
             * @param v Vector value
             */
            void setVec3(std::string name, glm::vec3 v) { m_vec3s[std::move(name)] = v; }
            
            /**
             * @brief Get all vec3 parameters.
             * @return Map of parameter names to vec3 values
             */
            const std::unordered_map<std::string, glm::vec3>& getVec3s() const { return m_vec3s; }

            /**
             * @brief Set a custom vec4 parameter.
             * @param name Parameter name (must match shader uniform)
             * @param v Vector value
             */
            void setVec4(std::string name, glm::vec4 v) { m_vec4s[std::move(name)] = v; }
            
            /**
             * @brief Get all vec4 parameters.
             * @return Map of parameter names to vec4 values
             */
            const std::unordered_map<std::string, glm::vec4>& getVec4s() const { return m_vec4s; }

            /**
             * @brief Get a specific float parameter.
             * @param name Parameter name
             * @return Optional containing the value if it exists
             */
            std::optional<float> getFloat(std::string name) {
                auto it = m_floats.find(name);
                if (it != m_floats.end()) {
                    return it->second;
                }
                return std::nullopt;
            }

            /**
             * @brief Get a specific vec3 parameter.
             * @param name Parameter name
             * @return Optional containing the value if it exists
             */
            std::optional<glm::vec3> getVec3(std::string name) {
                auto it = m_vec3s.find(name);
                if (it != m_vec3s.end()) {
                    return it->second;
                }
                return std::nullopt;
            }

            /**
             * @brief Get a specific vec4 parameter.
             * @param name Parameter name
             * @return Optional containing the value if it exists
             */
            std::optional<glm::vec4> getVec4(std::string name) {
                auto it = m_vec4s.find(name);
                if (it != m_vec4s.end()) {
                    return it->second;
                }
                return std::nullopt;
            }

            /**
             * @brief Create a copy of this material with a new name.
             * 
             * Useful for creating material variants or instances.
             * 
             * @param newName Name for the cloned material
             * @return Shared pointer to the cloned material
             */
            std::shared_ptr<Material> cloneWithName(std::string newName) const {
                auto copy = std::make_shared<Material>(*this);
                copy->m_name = std::move(newName);
                return copy;
            }

          private:
            std::string m_name;              ///< Material identifier
            std::string m_shaderName;        ///< Shader to use

            glm::vec4 m_baseColor{0.9f, 0.6f, 0.2f, 1.0f};  ///< Base/albedo color
            float m_pointSize{6.0f};                         ///< Point rendering size

            std::unordered_map<std::string, std::string> m_textures;  ///< Texture slot assignments
            std::unordered_map<std::string, float> m_floats;          ///< Custom float parameters
            std::unordered_map<std::string, glm::vec4> m_vec4s;      ///< Custom vec4 parameters
            std::unordered_map<std::string, glm::vec3> m_vec3s;      ///< Custom vec3 parameters
        };

    } // namespace Render
} // namespace Arche