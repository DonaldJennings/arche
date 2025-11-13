#pragma once
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Shader.h"

namespace Arche {
    namespace Render {

        // Renderer-agnostic material. Holds parameter values and a shader reference.
        // Textures are referenced by name (backend decides how to resolve/bind them).
        class Material {
          public:
            Material() = default;
            explicit Material(std::string name) : m_name(std::move(name)) {}

            const std::string& getName() const { return m_name; }

            void setShader(std::shared_ptr<Shader> shader) { m_shader = std::move(shader); }
            const std::shared_ptr<Shader>& getShader() const { return m_shader; }

            // Common parameters you can extend as needed
            void setBaseColor(const glm::vec4& c) { m_baseColor = c; }
            const glm::vec4& getBaseColor() const { return m_baseColor; }

            void setPointSize(float s) { m_pointSize = s; }
            float getPointSize() const { return m_pointSize; }

            // Named textures as symbolic references (e.g., "albedo", "normal")
            void setTexture(std::string slot, std::string textureName) { m_textures[std::move(slot)] = std::move(textureName); }
            const std::unordered_map<std::string, std::string>& getTextures() const { return m_textures; }

            // Arbitrary float/vec params
            void setFloat(std::string name, float v) { m_floats[std::move(name)] = v; }
            const std::unordered_map<std::string, float>& getFloats() const { return m_floats; }

            void setVec4(std::string name, glm::vec4 v) { m_vec4s[std::move(name)] = v; }
            const std::unordered_map<std::string, glm::vec4>& getVec4s() const { return m_vec4s; }

          private:
            std::string m_name;
            std::shared_ptr<Shader> m_shader;

            glm::vec4 m_baseColor{0.9f, 0.6f, 0.2f, 1.0f};
            float m_pointSize{6.0f};

            std::unordered_map<std::string, std::string> m_textures;
            std::unordered_map<std::string, float> m_floats;
            std::unordered_map<std::string, glm::vec4> m_vec4s;
        };

    } // namespace Render
} // namespace Arche