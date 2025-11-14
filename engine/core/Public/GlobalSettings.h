#pragma once

#include <glm/glm.hpp>

namespace Arche {
    namespace Core {

        struct RenderSettings {
            glm::vec4 clearColor{0.1f, 0.12f, 0.15f, 1.0f};
            float fieldOfView{60.0f};
            bool vsync{true};
            glm::vec3 sunPosition{10.0f, 10.0f, 0.0f};
            glm::vec3 sunColor{1.0f, 1.0f, 0.9f};

            bool wireframe{false};
            bool showGrid{true};
            bool showAxes{true};
        };

        struct WorldSettings {
            glm::vec3 gravity{0.0f, -9.81f, 0.0f};
            float timeScale{1.0f};
        };

        class GlobalSettings {
        public:
            RenderSettings& getRenderSettings() { return m_renderSettings; }
            const RenderSettings& getRenderSettings() const { return m_renderSettings; }

            WorldSettings& getWorldSettings() { return m_worldSettings; }
            const WorldSettings& getWorldSettings() const { return m_worldSettings; }

        private:
            RenderSettings m_renderSettings;
            WorldSettings m_worldSettings;
        };
    }
}