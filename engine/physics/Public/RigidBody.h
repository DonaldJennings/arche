#pragma once
#include <glm/glm.hpp>
#include <memory>

namespace Arche {
    namespace Physics {

        class RigidBody {
          public:
            RigidBody(float mass)
                : m_Mass(mass), m_Velocity(0.0f), m_Acceleration(0.0f), m_useGravity(true), m_isStatic(false) {}

            float getMass() const { return m_Mass; }
            void setMass(float mass) { m_Mass = mass; }

            glm::vec3 getVelocity() const { return m_Velocity; }
            glm::vec3 getAcceleration() const { return m_Acceleration; }

            void setVelocity(const glm::vec3 &velocity) { m_Velocity = velocity; }
            void setAcceleration(const glm::vec3 &acceleration) { m_Acceleration = acceleration; }

            bool isUsingGravity() const { return m_useGravity; }
            void setUseGravity(bool useGravity) { m_useGravity = useGravity; }
            bool isStatic() const { return m_isStatic; }
            void setStatic(bool isStatic) { m_isStatic = isStatic; }

            void applyForce(const glm::vec3 &force) {
                if (m_isStatic)
                    return;
                m_Acceleration += force / m_Mass;
            }

            void integrate(float deltaTime, glm::vec3& position, glm::vec3 gravity) {
                if (m_isStatic)
                    return;

                if (m_useGravity)
                    applyForce(gravity * m_Mass);

                m_Velocity += m_Acceleration * deltaTime;
                position += m_Velocity * deltaTime;
                m_Acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
            }

          private:
            float m_Mass{};
            glm::vec3 m_Velocity{0.0f, 0.0f, 0.0f};
            glm::vec3 m_Acceleration{0.0f, 0.0f, 0.0f};
            bool m_useGravity{true};
            bool m_isStatic{false};
        };

    } // namespace Physics
} // namespace Arche