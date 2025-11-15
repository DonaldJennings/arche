#pragma once
#include "ICollider.h"

namespace Arche {
    namespace Physics {
        class PlaneCollider : public ICollider {
          public:
            PlaneCollider(const glm::vec3 &normal, float distance)
                : m_Normal(glm::normalize(normal)), m_Distance(distance) {}

            Type GetType() const override { return Type::Plane; }

            bool hasCollision(const ICollider &other, glm::vec3 &normal, float &penetration) const override {
                return false;
            };

            const glm::vec3 &GetNormal() const { return m_Normal; }
            float GetDistance() const { return m_Distance; }

            void SetNormal(const glm::vec3 &normal) { m_Normal = glm::normalize(normal); }
            void SetDistance(float distance) { m_Distance = distance; }

          private:
            glm::vec3 m_Normal;
            float m_Distance;
        };
    } // namespace Physics
} // namespace Arche