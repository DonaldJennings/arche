#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Arche {
    namespace Physics {

        /**
         * @brief Abstract interface for collision shapes.
         * 
         * This interface defines the contract for all collider types in the physics
         * system. Colliders represent the geometric shape used for collision detection
         * and response. Each collider type (sphere, plane, cube) implements this
         * interface to provide type-specific collision detection algorithms.
         * 
         * @note Collision detection is performed through double-dispatch using the
         *       hasCollision method and type queries.
         */
        class ICollider {
          public:
            /**
             * @brief Enumeration of supported collider types.
             * 
             * Used for type identification and collision algorithm selection.
             */
            enum class Type { 
                Sphere,  ///< Spherical collision shape
                Plane,   ///< Infinite planar collision shape
                Cube     ///< Axis-aligned box collision shape
            };

            /**
             * @brief Virtual destructor for proper cleanup.
             */
            virtual ~ICollider() = default;
            
            /**
             * @brief Get the collider type.
             * 
             * @return The Type enumeration value for this collider
             */
            virtual Type GetType() const = 0;

            /**
             * @brief Check for collision with another collider.
             * 
             * Determines if this collider intersects with another collider and
             * computes collision response information if they do collide.
             * 
             * @param other The other collider to test against
             * @param contactNormal Output parameter for the collision normal vector
             * @param penetrationDepth Output parameter for how deeply the colliders overlap
             * @return true if collision detected, false otherwise
             * 
             * @note The contact normal points from this collider toward the other
             */
            virtual bool hasCollision(const ICollider &other, glm::vec3& contactNormal, float& penetrationDepth) const = 0;
        };
    } // namespace Physics
} // namespace Arche