
#ifndef ARCHE_ENGINE_CONFIGURATION_H
#define ARCHE_ENGINE_CONFIGURATION_H

#include <Vector3D.h>

namespace Arche {
    namespace Core {
        struct WorldConfig {
            Math::Vector3D gravity = Math::Vector3D(0.0f, -9.81f, 0.0f); // Gravity vector
            float stepDuration = 1.0f / 60.0f; // Fixed timestep duration in seconds
            int maxSubSteps = 5; // Maximum number of sub-steps per frame
            bool deterministic = true; // Use deterministic updates
        };
    } // namespace Core
} // namespace Arche
#endif // ARCHE_ENGINE_CONFIGURATION_H