#pragma once

#include "ICollider.h"

namespace Arche {
    namespace Physics {
        class SphereCollider : public ICollider {
          public:
            SphereCollider(float radius, glm::vec3 *position) : m_Radius(radius), m_Position(position) {}

            Type GetType() const override { return Type::Sphere; }

            bool hasCollision(const ICollider &other, glm::vec3 &normal, float &penetration) const override {
                return false;
            };

            float GetRadius() const { return m_Radius; }
            const glm::vec3 &GetPosition() const { return *m_Position; }

          private:
            float m_Radius;
            glm::vec3 *m_Position;
        };
    } // namespace Physics

} // namespace Arche