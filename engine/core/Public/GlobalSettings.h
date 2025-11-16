#pragma once

#include <glm/glm.hpp>

namespace Arche {
    namespace Core {

        /**
         * @brief Rendering configuration settings for the engine.
         * 
         * This structure contains all rendering-related settings including
         * viewport colors, field of view, vsync, lighting properties, and
         * debug visualization options.
         */
        struct RenderSettings {
            glm::vec4 clearColor{0.1f, 0.12f, 0.15f, 1.0f}; ///< Background clear color (RGBA)
            float fieldOfView{60.0f};                        ///< Camera field of view in degrees
            bool vsync{true};                                ///< Enable vertical synchronization

            glm::vec3 ambientLight{0.2f, 0.2f, 0.2f};       ///< Ambient light color
            float ambientStrength{0.5f};                     ///< Ambient light strength multiplier
            glm::vec3 ambientPosition{10.0f, 10.0f, 0.0f};  ///< Ambient light position

            glm::vec3 directionalLightColor{1.0f, 1.0f, 1.0f};          ///< Directional light color
            glm::vec3 directionalLightDirection{-1.0f, -1.0f, -1.0f};   ///< Directional light direction vector
            glm::vec3 directionalLightIntensity{1.0f, 1.0f, 1.0f};      ///< Directional light intensity
            float directionalLightDistance{100.0f};                      ///< Directional light maximum distance

            bool wireframe{false};  ///< Enable wireframe rendering mode
            bool showGrid{true};    ///< Show grid in viewport
            bool showAxes{true};    ///< Show coordinate axes in viewport
        };

        /**
         * @brief Physics and world simulation settings.
         * 
         * This structure contains global settings that affect the physics
         * simulation and world behavior.
         */
        struct WorldSettings {
            glm::vec3 gravity{0.0f, -9.81f, 0.0f};  ///< Gravity acceleration vector (m/s²)
            float timeScale{1.0f};                   ///< Time scale multiplier for simulation speed
        };

        /**
         * @brief Global settings container for the entire engine.
         * 
         * This class provides centralized access to all global configuration
         * settings, including rendering and world/physics settings. It serves
         * as a single source of truth for engine-wide configuration.
         */
        class GlobalSettings {
        public:
            /**
             * @brief Get mutable reference to render settings.
             * @return Reference to the render settings structure
             */
            RenderSettings& getRenderSettings() { return m_renderSettings; }
            
            /**
             * @brief Get const reference to render settings.
             * @return Const reference to the render settings structure
             */
            const RenderSettings& getRenderSettings() const { return m_renderSettings; }

            /**
             * @brief Get mutable reference to world settings.
             * @return Reference to the world settings structure
             */
            WorldSettings& getWorldSettings() { return m_worldSettings; }
            
            /**
             * @brief Get const reference to world settings.
             * @return Const reference to the world settings structure
             */
            const WorldSettings& getWorldSettings() const { return m_worldSettings; }

        private:
            RenderSettings m_renderSettings;  ///< Rendering configuration
            WorldSettings m_worldSettings;    ///< World and physics configuration
        };
    }
}