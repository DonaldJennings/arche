#pragma once
#include "PlaneCollider.h"
#include "IEntity.h"
#include <glm/glm.hpp>
#include <memory>

namespace Arche {

    namespace Scene {
        /**
         * @brief Entity representing an infinite planar surface.
         * 
         * PlaneEntity is a concrete implementation of IEntity for flat surfaces
         * like floors and walls. It includes a plane collider but no rigid body
         * since planes are always static. The plane is defined by a normal vector
         * and distance from the origin.
         * 
         * Planes are useful for boundaries, floors, and walls that don't need
         * complex collision shapes.
         */
        class PlaneEntity : public IEntity {
          public:
            /**
             * @brief Construct a plane entity.
             * 
             * Creates an infinite plane defined by a normal vector and distance
             * from the origin. The position is computed as normal * distance.
             * 
             * @param normal The plane's normal vector (will be normalized)
             * @param distance Signed distance from origin along the normal
             */
            PlaneEntity(const glm::vec3 &normal, float distance)
                : m_Normal(glm::normalize(normal)), m_Distance(distance) {
                m_Position = m_Normal * distance;
                m_Rotation = glm::vec3(0.0f);
                m_Scale = glm::vec3(1.0f);
                m_Collider = std::make_shared<Physics::PlaneCollider>(m_Normal, m_Distance);
            }

            // --- Transform ---
            glm::vec3 getPosition() const override { return m_Position; }
            glm::vec3 *getPositionPtr() override { return &m_Position; }
            
            /**
             * @brief Set the plane's position.
             * 
             * Updates both the position and the distance value in the collider
             * to keep them synchronized.
             * 
             * @param pos New position in world space
             */
            void setPosition(const glm::vec3 &pos) override {
                m_Position = pos;
                m_Distance = glm::dot(m_Normal, m_Position);

                if (m_Collider) {
                    if (auto planeCollider = std::dynamic_pointer_cast<Physics::PlaneCollider>(m_Collider)) {
                        planeCollider->SetDistance(m_Distance);
                    }
                }
            }

            glm::vec3 getRotation() const override { return m_Rotation; }
            void setRotation(const glm::vec3 &rot) override { m_Rotation = rot; }

            glm::vec3 getScale() const override { return m_Scale; }
            void setScale(const glm::vec3 &scale) override { m_Scale = scale; }

            // --- Lifecycle ---
            /**
             * @brief Update the plane entity for one frame.
             * 
             * Planes are static and don't need per-frame updates.
             * 
             * @param deltaTime Time since last frame in seconds (unused)
             */
            void update(double deltaTime) override {
                // Static entity -- no per-frame updates
            }

            // --- Components ---
            std::shared_ptr<Physics::RigidBody> getRigidBody() override { return m_Rigidbody; }
            std::shared_ptr<Physics::ICollider> getCollider() override { return m_Collider; }
            
            std::string_view getMeshId() override { return m_meshName; }
            std::string_view getMaterialId() override { return m_materialName; }
            void setMaterialId(std::string_view materialId) override { m_materialName = std::string(materialId); }
            std::string_view getShaderId() override { return "flat.color"; }

            // --- Identification ---
            std::uint64_t getID() const override { return m_id; }
            void setID(std::uint64_t id) override { m_id = id; };
            std::string_view getName() const override { return m_name; }

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
             * @brief Create a deep copy of this plane entity.
             * 
             * @return Shared pointer to the cloned plane with same properties
             */
            std::shared_ptr<IEntity> clone() const override {
                auto cloned{std::make_shared<PlaneEntity>(m_Normal, glm::dot(m_Normal, m_Position))};
                cloned->setPosition(m_Position);
                cloned->setRotation(m_Rotation);
                cloned->setScale(m_Scale);
                cloned->setID(m_id);
                cloned->setMaterialId(m_Renderable->getMaterialName());
                return cloned;
            }

          private:
            std::uint64_t m_id{};                                           ///< Unique entity ID

            std::string m_name{"plane"};                                    ///< Entity name
            glm::vec3 m_Normal;                                             ///< Plane normal vector
            float m_Distance;                                               ///< Distance from origin
            glm::vec3 m_Position;                                           ///< World position
            glm::vec3 m_Rotation;                                           ///< Euler rotation (degrees)
            glm::vec3 m_Scale;                                              ///< Scale factors

            std::shared_ptr<Physics::ICollider> m_Collider;                ///< Collision shape
            std::shared_ptr<Render::IRenderable> m_Renderable;             ///< Rendering component
            std::shared_ptr<Physics::RigidBody> m_Rigidbody;               ///< Physics body (always null)
            std::string m_meshName{"plane.mesh"};                           ///< Mesh resource name
            std::string m_materialName{"plane.mat"};                        ///< Material resource name
        };
    } // namespace Scene
} // namespace Arche
