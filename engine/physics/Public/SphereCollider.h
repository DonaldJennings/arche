#pragma once

#include "ICollider.h"

namespace Arche {
    namespace Physics {
        /**
         * @brief Spherical collider for ball-shaped objects.
         * 
         * SphereCollider represents a perfect sphere collision shape defined by
         * a radius and center position. The collider maintains a pointer to its
         * position which is updated by the physics system or entity owner.
         * 
         * Spheres are one of the simplest and most efficient collision shapes,
         * making them ideal for projectiles, particles, and round objects.
         * 
         * @note This implementation currently returns false for all collision
         *       checks (collision detection not yet implemented).
         */
        class SphereCollider : public ICollider {
          public:
            /**
             * @brief Construct a sphere collider.
             * 
             * @param radius The radius of the sphere (must be positive)
             * @param position Pointer to the center position (must remain valid)
             * 
             * @note The position pointer is not owned by the collider and must
             *       remain valid for the lifetime of the collider.
             */
            SphereCollider(float radius, glm::vec3 *position) : m_Radius(radius), m_Position(position) {}

            /**
             * @brief Get the collider type.
             * @return Type::Sphere
             */
            Type GetType() const override { return Type::Sphere; }

            /**
             * @brief Check for collision with another collider.
             * 
             * @param other The other collider to test against
             * @param normal Output parameter for collision normal (unused)
             * @param penetration Output parameter for penetration depth (unused)
             * @return false (collision detection not implemented)
             * 
             * @note This is a placeholder implementation that always returns false.
             */
            bool hasCollision(const ICollider &other, glm::vec3 &normal, float &penetration) const override {
                return false;
            };

            /**
             * @brief Get the sphere's radius.
             * @return Radius in world units
             */
            float GetRadius() const { return m_Radius; }
            
            /**
             * @brief Get the sphere's center position.
             * @return Reference to the position vector
             */
            const glm::vec3 &GetPosition() const { return *m_Position; }

          private:
            float m_Radius;         ///< Radius of the sphere
            glm::vec3 *m_Position;  ///< Pointer to center position (not owned)
        };
    } // namespace Physics

} // namespace Arche