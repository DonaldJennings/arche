#pragma once
#include "ICollider.h"

namespace Arche {
    namespace Physics {
        /**
         * @brief Infinite planar collider for ground and wall surfaces.
         * 
         * PlaneCollider represents an infinite flat plane defined by a normal
         * vector and a signed distance from the origin. The plane divides space
         * into two half-spaces, with the normal pointing toward the "positive"
         * side.
         * 
         * Planes are typically used for floors, walls, and other large flat
         * surfaces that can be considered infinite for collision purposes.
         * 
         * The plane equation is: normal · point - distance = 0
         * 
         * @note This implementation currently returns false for all collision
         *       checks (collision detection not yet implemented).
         */
        class PlaneCollider : public ICollider {
          public:
            /**
             * @brief Construct a plane collider.
             * 
             * @param normal The plane's normal vector (will be normalized)
             * @param distance Signed distance from origin along the normal
             * 
             * @note The normal vector will automatically be normalized during construction.
             */
            PlaneCollider(const glm::vec3 &normal, float distance)
                : m_Normal(glm::normalize(normal)), m_Distance(distance) {}

            /**
             * @brief Get the collider type.
             * @return Type::Plane
             */
            Type GetType() const override { return Type::Plane; }

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
             * @brief Get the plane's normal vector.
             * @return Normalized normal vector
             */
            const glm::vec3 &GetNormal() const { return m_Normal; }
            
            /**
             * @brief Get the plane's distance from origin.
             * @return Signed distance value
             */
            float GetDistance() const { return m_Distance; }

            /**
             * @brief Set a new normal vector for the plane.
             * 
             * @param normal The new normal vector (will be normalized)
             */
            void SetNormal(const glm::vec3 &normal) { m_Normal = glm::normalize(normal); }
            
            /**
             * @brief Set a new distance from origin.
             * 
             * @param distance The new signed distance value
             */
            void SetDistance(float distance) { m_Distance = distance; }

          private:
            glm::vec3 m_Normal;  ///< Normalized plane normal vector
            float m_Distance;    ///< Signed distance from origin
        };
    } // namespace Physics
} // namespace Arche