#pragma once

#include "ICollider.h"

namespace Arche {
    namespace Physics {
        /**
         * @brief Spherical collider for ball-shaped objects.
         *
         * Defined only by radius. Position is managed by PhysicsSystem's SoA
         * arrays and does not need to be stored here.
         */
        class SphereCollider : public ICollider {
          public:
            explicit SphereCollider(float radius) : m_Radius(radius) {}

            Type GetType() const override { return Type::Sphere; }

            bool hasCollision(const ICollider &other, glm::vec3 &normal, float &penetration) const override {
                return false;
            }

            float GetRadius() const { return m_Radius; }

          private:
            float m_Radius;
        };
    } // namespace Physics
} // namespace Arche
