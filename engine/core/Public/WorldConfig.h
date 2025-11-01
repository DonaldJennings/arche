#pragma once

#include "../math/Vector3D.h"
namespace Arche {
    namespace Core {
        struct WorldConfig {
            Math::Vector3D gravity{0.0f, -9.81f, 0.0f}; // Default gravity pointing downwards
            float stepDuration{1.0f / 60.0f};           // Default to 60 Hz
            int maxSubSteps{5};                         // Maximum number of sub-steps per update
            bool deterministic{true};                   // Whether the simulation should be deterministic
        };
    } // namespace Core
} // namespace Arche