#pragma once
#include "ICollider.h"

namespace Arche {
    namespace Physics {
        class CubeCollider : public ICollider {
          public:
            CubeCollider(const glm::vec3 &halfExtents, const glm::vec3 *position)
                : m_HalfExtents(halfExtents), m_Position(position) {}
            Type GetType() const override { return Type::Cube; }

            bool hasCollision(const ICollider &other, glm::vec3 &normal, float &penetration) const override {
                return false;
            };

            const glm::vec3 &GetHalfExtents() const { return m_HalfExtents; }
            const glm::vec3 *GetPosition() const { return m_Position; }

          private:
            glm::vec3 m_HalfExtents;
            const glm::vec3 *m_Position;
        };
    } // namespace Physics
} // namespace Arche