#pragma once
#include "ICollider.h"

namespace Arche {
    namespace Physics {
        /**
         * @brief Axis-aligned box collider for cube-shaped objects.
         *
         * Defined only by half-extents. Position is managed by PhysicsSystem's
         * SoA arrays and does not need to be stored here.
         */
        class CubeCollider : public ICollider {
          public:
            explicit CubeCollider(const glm::vec3 &halfExtents) : m_HalfExtents(halfExtents) {}

            Type GetType() const override { return Type::Cube; }

            bool hasCollision(const ICollider &other, glm::vec3 &normal, float &penetration) const override {
                return false;
            }

            const glm::vec3 &GetHalfExtents() const { return m_HalfExtents; }

          private:
            glm::vec3 m_HalfExtents;
        };
    } // namespace Physics
} // namespace Arche
