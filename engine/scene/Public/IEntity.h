#pragma once

#include "ICollider.h"
#include "IRenderable.h"
#include "RigidBody.h"

#include <glm/glm.hpp>
#include <memory>
#include <string_view>

namespace Arche {
    namespace Scene {
        /**
         * @brief Abstract interface for all entities in the game world.
         * 
         * IEntity defines the contract for objects that exist in the 3D world.
         * Entities have transforms (position, rotation, scale), can optionally
         * have physics components (rigid body, collider), and visual representation
         * (mesh, material). This interface allows the engine to treat all game
         * objects uniformly while allowing type-specific implementations.
         * 
         * All entities must be cloneable to support features like state snapshots
         * and entity duplication.
         */
        class IEntity {
          public:
            /**
             * @brief Virtual destructor for proper cleanup.
             */
            virtual ~IEntity() = default;

            /**
             * @brief Get the unique identifier for this entity.
             * @return Entity ID (unique within the world)
             */
            virtual std::uint64_t getID() const = 0;
            
            /**
             * @brief Set the unique identifier for this entity.
             * @param id The new entity ID
             * 
             * @note Usually set by the WorldSystem when the entity is added
             */
            virtual void setID(std::uint64_t id) = 0;

            /**
             * @brief Get the entity's world position.
             * @return Position vector in world space
             */
            virtual glm::vec3 getPosition() const = 0;
            
            /**
             * @brief Set the entity's world position.
             * @param position New position in world space
             */
            virtual void setPosition(const glm::vec3 &position) = 0;

            /**
             * @brief Get the entity's rotation.
             * @return Rotation as Euler angles (in degrees)
             */
            virtual glm::vec3 getRotation() const = 0;
            
            /**
             * @brief Set the entity's rotation.
             * @param rotation New rotation as Euler angles (in degrees)
             */
            virtual void setRotation(const glm::vec3 &rotation) = 0;

            /**
             * @brief Get the entity's scale.
             * @return Scale vector (1.0 = normal size)
             */
            virtual glm::vec3 getScale() const = 0;
            
            /**
             * @brief Set the entity's scale.
             * @param scale New scale vector
             */
            virtual void setScale(const glm::vec3 &scale) = 0;

            /**
             * @brief Update the entity for one frame.
             * 
             * Called each frame to allow entities to update their internal state,
             * run animations, or perform entity-specific logic.
             * 
             * @param deltaTime Time elapsed since last update in seconds
             */
            virtual void update(double deltaTime) = 0;

            /**
             * @brief Get the entity's rigid body physics component.
             * @return Shared pointer to RigidBody, or nullptr if no physics
             */
            virtual std::shared_ptr<Physics::RigidBody> getRigidBody() = 0;
            
            /**
             * @brief Get the entity's collision shape component.
             * @return Shared pointer to ICollider, or nullptr if no collision
             */
            virtual std::shared_ptr<Physics::ICollider> getCollider() = 0;

            /**
             * @brief Get the mesh identifier for rendering.
             * @return String view of the mesh resource name
             */
            virtual std::string_view getMeshId() = 0;
            
            /**
             * @brief Get the material identifier for rendering.
             * @return String view of the material resource name
             */
            virtual std::string_view getMaterialId() = 0;
            
            /**
             * @brief Set the material identifier.
             * @param materialId New material resource name
             */
            virtual void setMaterialId(std::string_view materialId) = 0;
            
            /**
             * @brief Get the shader identifier for rendering.
             * @return String view of the shader resource name
             */
            virtual std::string_view getShaderId() = 0;
            
            /**
             * @brief Get the human-readable name of this entity.
             * @return String view of the entity name
             */
            virtual std::string_view getName() const = 0;
            
            /**
             * @brief Create a deep copy of this entity.
             * 
             * The cloned entity should have the same properties and state
             * as the original, but be a separate instance. Used for entity
             * duplication and state snapshots.
             * 
             * @return Shared pointer to the cloned entity
             */
            virtual std::shared_ptr<IEntity> clone() const = 0;
        };
    } // namespace Scene
} // namespace Arche