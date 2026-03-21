#pragma once
#include "CubeCollider.h"
#include "ICollider.h"
#include "IEntity.h"
#include "Rigidbody.h"

namespace Arche {
    namespace Scene {
        /**
         * @brief Entity representing a cube/box object in the world.
         * 
         * CubeEntity is a concrete implementation of IEntity for rectangular
         * box-shaped objects. It includes a cube collider and rigid body for
         * physics simulation, and uses a standard cube mesh for rendering.
         * 
         * The cube is defined by its half-extents (half-width, half-height,
         * half-depth from the center), which allows for easy scaling and
         * collision detection.
         */
        class CubeEntity : public IEntity {
          public:
            /**
             * @brief Construct a cube entity.
             * 
             * Creates a cube with the specified size and position. Optionally
             * enables physics simulation (rigid body dynamics and collision).
             * 
             * @param halfExtents Half the width, height, and depth of the cube
             * @param position Initial world position
             * @param physicsEnabled If true, creates rigid body and collider
             */
            CubeEntity(const glm::vec3 &halfExtents, const glm::vec3 &position, bool physicsEnabled = true)
                : m_Position(position), m_Rotation(0.0f), m_Scale(1.0f), m_HalfExtents(halfExtents),
                  m_Renderable{std::make_shared<Render::NamedRenderable>("cube.mesh", "cube.mat")} {
                if (physicsEnabled) {
                    m_Rigidbody = std::make_shared<Physics::RigidBody>(1.0f);
                    m_Collider = std::make_shared<Physics::CubeCollider>(halfExtents);
                }
            }

            // --- Transform ---
            glm::vec3 getPosition() const override { return m_Position; }
            void setPosition(const glm::vec3 &pos) override { m_Position = pos; }

            glm::vec3 getRotation() const override { return m_Rotation; }
            void setRotation(const glm::vec3 &rot) override { m_Rotation = rot; }

            glm::vec3 getScale() const override { return m_Scale; }
            void setScale(const glm::vec3 &scale) override { m_Scale = scale; }

            // --- Lifecycle ---
            /**
             * @brief Update the cube entity for one frame.
             * 
             * Currently a no-op for cubes. Override to add cube-specific
             * behavior like animations or state updates.
             * 
             * @param deltaTime Time since last frame in seconds
             */
            void update(double deltaTime) override {
                // Optional cube-specific logic
            }

            // --- Components ---
            std::shared_ptr<Physics::RigidBody> getRigidBody() override { return m_Rigidbody; }
            std::shared_ptr<Physics::ICollider> getCollider() override { return m_Collider; }

            std::string_view getMeshId() override { return m_Renderable->getMeshName(); }
            std::string_view getMaterialId() override { return m_Renderable->getMaterialName(); }
            void setMaterialId(std::string_view materialId) override {
                if (m_Renderable) {
                    m_Renderable->setMaterialName(std::string(materialId));
                }
            }
            std::string_view getShaderId() override { return "flat.color"; }

            // --- Identification ---
            std::uint64_t getID() const override { return m_id; }
            void setID(std::uint64_t id) override { m_id = id; };
            std::string_view getName() const override { return m_name; }

            /**
             * @brief Get the cube's half-extents.
             * @return Vector containing half-width, half-height, half-depth
             */
            const glm::vec3 &GetHalfExtents() const { return m_HalfExtents; }

            /**
             * @brief Create a deep copy of this cube entity.
             * 
             * @return Shared pointer to the cloned cube with same properties
             */
            std::shared_ptr<IEntity> clone() const override {
                auto cloned{std::make_shared<CubeEntity>(m_HalfExtents, m_Position)};
                cloned->setID(m_id);
                cloned->setMaterialId(m_Renderable->getMaterialName());

                return cloned;
            }

          private:
            std::uint64_t m_id{};                                           ///< Unique entity ID

            std::string m_name{"cube"};                                     ///< Entity name
            glm::vec3 m_Position;                                           ///< World position
            glm::vec3 m_Rotation;                                           ///< Euler rotation (degrees)
            glm::vec3 m_Scale;                                              ///< Scale factors
            glm::vec3 m_HalfExtents;                                        ///< Half the cube dimensions

            std::shared_ptr<Physics::RigidBody> m_Rigidbody;               ///< Physics body (optional)
            std::shared_ptr<Physics::ICollider> m_Collider;                ///< Collision shape (optional)
            std::shared_ptr<Render::NamedRenderable> m_Renderable;         ///< Rendering component
        };
    } // namespace Scene
} // namespace Arche