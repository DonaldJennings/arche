#pragma once
#include <glm/glm.hpp>
#include <memory>

namespace Arche {
    namespace Physics {

        /**
         * @brief Represents a dynamic physical body in the physics simulation.
         * 
         * RigidBody implements basic Newtonian physics for objects in the world.
         * It tracks mass, velocity, acceleration, and provides integration methods
         * for updating position based on forces. Bodies can be marked as static
         * to prevent physics simulation, and can optionally be affected by gravity.
         * 
         * The physics uses a simple Euler integration scheme where:
         * - Forces are accumulated into acceleration (F = ma)
         * - Acceleration is integrated into velocity
         * - Velocity is integrated into position
         * - Acceleration is reset each frame
         */
        class RigidBody {
          public:
            /**
             * @brief Construct a rigid body with the specified mass.
             * 
             * Creates a body at rest with zero velocity and acceleration.
             * Gravity is enabled by default and the body is dynamic (not static).
             * 
             * @param mass The mass of the body in kilograms (must be positive)
             */
            RigidBody(float mass)
                : m_Mass(mass), m_Velocity(0.0f), m_Acceleration(0.0f), m_useGravity(true), m_isStatic(false) {}

            /**
             * @brief Get the mass of the rigid body.
             * @return Mass in kilograms
             */
            float getMass() const { return m_Mass; }
            
            /**
             * @brief Set the mass of the rigid body.
             * @param mass New mass value in kilograms
             */
            void setMass(float mass) { m_Mass = mass; }

            /**
             * @brief Get the current velocity.
             * @return Velocity vector in meters per second
             */
            glm::vec3 getVelocity() const { return m_Velocity; }
            
            /**
             * @brief Get the current acceleration.
             * @return Acceleration vector in meters per second squared
             */
            glm::vec3 getAcceleration() const { return m_Acceleration; }

            /**
             * @brief Set the velocity directly.
             * @param velocity New velocity vector in meters per second
             */
            void setVelocity(const glm::vec3 &velocity) { m_Velocity = velocity; }
            
            /**
             * @brief Set the acceleration directly.
             * @param acceleration New acceleration vector in meters per second squared
             */
            void setAcceleration(const glm::vec3 &acceleration) { m_Acceleration = acceleration; }

            /**
             * @brief Check if gravity is being applied to this body.
             * @return true if gravity affects this body, false otherwise
             */
            bool isUsingGravity() const { return m_useGravity; }
            
            /**
             * @brief Enable or disable gravity for this body.
             * @param useGravity true to enable gravity, false to disable
             */
            void setUseGravity(bool useGravity) { m_useGravity = useGravity; }
            
            /**
             * @brief Check if this body is static (immovable).
             * @return true if static, false if dynamic
             */
            bool isStatic() const { return m_isStatic; }
            
            /**
             * @brief Set whether this body is static.
             * 
             * Static bodies do not respond to forces or gravity and remain
             * fixed in place. Useful for walls, floors, and other immovable objects.
             * 
             * @param isStatic true to make static, false to make dynamic
             */
            void setStatic(bool isStatic) { m_isStatic = isStatic; }

            /**
             * @brief Apply a force to the rigid body.
             * 
             * The force is converted to acceleration using F = ma and accumulated
             * into the current acceleration. Has no effect on static bodies.
             * 
             * @param force Force vector in Newtons
             */
            void applyForce(const glm::vec3 &force) {
                if (m_isStatic)
                    return;
                m_Acceleration += force / m_Mass;
            }

            /**
             * @brief Integrate physics for one timestep.
             * 
             * Applies gravity (if enabled), integrates acceleration into velocity,
             * integrates velocity into position, and resets acceleration for the
             * next frame. Has no effect on static bodies.
             * 
             * @param deltaTime Time step in seconds
             * @param position Reference to the position vector to update
             * @param gravity Gravity acceleration vector from world settings
             */
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
            float m_Mass{};                             ///< Mass in kilograms
            glm::vec3 m_Velocity{0.0f, 0.0f, 0.0f};    ///< Velocity in m/s
            glm::vec3 m_Acceleration{0.0f, 0.0f, 0.0f};///< Acceleration in m/s²
            bool m_useGravity{true};                    ///< Whether gravity affects this body
            bool m_isStatic{false};                     ///< Whether this body is immovable
        };

    } // namespace Physics
} // namespace Arche