#pragma once
#include "ICollider.h"

namespace Arche {
    namespace Physics {
        /**
         * @brief Axis-aligned box collider for cube-shaped objects.
         * 
         * CubeCollider represents a rectangular box collision shape defined by
         * its half-extents (half the width, height, and depth). The collider
         * maintains a pointer to its position which is updated by the physics
         * system or entity owner.
         * 
         * @note This implementation currently returns false for all collision
         *       checks (collision detection not yet implemented).
         */
        class CubeCollider : public ICollider {
          public:
            /**
             * @brief Construct a cube collider.
             * 
             * @param halfExtents Half-width, half-height, and half-depth of the cube
             * @param position Pointer to the position vector (must remain valid)
             * 
             * @note The position pointer is not owned by the collider and must
             *       remain valid for the lifetime of the collider.
             */
            CubeCollider(const glm::vec3 &halfExtents, const glm::vec3 *position)
                : m_HalfExtents(halfExtents), m_Position(position) {}
            
            /**
             * @brief Get the collider type.
             * @return Type::Cube
             */
            Type GetType() const override { return Type::Cube; }

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
             * @brief Get the half-extents of the cube.
             * @return Vector containing half-width, half-height, and half-depth
             */
            const glm::vec3 &GetHalfExtents() const { return m_HalfExtents; }
            
            /**
             * @brief Get the position pointer.
             * @return Pointer to the cube's center position
             */
            const glm::vec3 *GetPosition() const { return m_Position; }

          private:
            glm::vec3 m_HalfExtents;      ///< Half the dimensions of the cube
            const glm::vec3 *m_Position;  ///< Pointer to center position (not owned)
        };
    } // namespace Physics
} // namespace Arche