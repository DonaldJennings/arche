#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Arche {
    namespace Physics {

        class ICollider {
          public:
            enum class Type { Sphere, Plane, Cube };

            virtual ~ICollider() = default;
            virtual Type GetType() const = 0;

            virtual bool hasCollision(const ICollider &other, glm::vec3& contactNormal, float& penetrationDepth) const = 0;
        };
    } // namespace Physics
} // namespace Arche