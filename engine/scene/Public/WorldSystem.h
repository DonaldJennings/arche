#pragma once
#include "IEntity.h"
#include <ISubsystem.h>
#include <LoggingService.h>
#include <PhysicsSystem.h>
#include <TimingService.h>
#include <memory>
#include <optional>

namespace Arche {
    namespace Scene {

        /**
         * @brief View snapshot of the world state.
         * 
         * Provides a read-only view of all entities in the world at a
         * specific moment. Used by rendering and other systems that need
         * to iterate over all entities.
         */
        class WorldSystemView {
          public:
            std::vector<std::shared_ptr<IEntity>> bodies;  ///< List of all entities
        };

        /**
         * @brief Snapshot of an entity's state for save/restore.
         * 
         * Stores the entity ID and a prototype clone for restoring the
         * world to a previous state (e.g., simulation reset).
         */
        struct EntitySnapshot {
            std::uint64_t id{0};                    ///< Entity ID
            std::shared_ptr<IEntity> prototype;     ///< Cloned entity state
        };

        /**
         * @brief Main scene management subsystem.
         * 
         * WorldSystem manages all entities in the game world. It handles
         * entity creation, deletion, updates, and property modifications.
         * The system also supports state snapshots for simulation replay/reset.
         * 
         * WorldSystem coordinates with the PhysicsSystem to ensure entities
         * with physics components are properly simulated. It assigns unique
         * IDs to all entities and maintains them in an efficient lookup table.
         * 
         * Key features:
         * - Entity lifecycle management (add, remove, update)
         * - State snapshot and restoration for simulation reset
         * - Integration with physics simulation
         * - Property update methods for common entity modifications
         */
        class WorldSystem : public Core::ISubsystem {
          public:
            /**
             * @brief Construct the world system.
             * 
             * @param logger Shared pointer to logging service
             * @param timing Shared pointer to timing service
             */
            WorldSystem(std::shared_ptr<Arche::Core::LoggingService> logger,
                        std::shared_ptr<Arche::Core::TimingService> timing);

            /**
             * @brief Initialize the world system.
             * 
             * Currently a no-op as the world system doesn't require
             * initialization beyond construction.
             */
            void initialise() override {};

            /**
             * @brief Update all entities and physics for one frame.
             * 
             * Updates the physics system first, then updates each entity's
             * internal state. The delta time is passed through to both systems.
             * 
             * @param dt Time elapsed since last update in seconds
             */
            void update(double dt) override {
                if (m_physicsSystem) {
                    m_physicsSystem->update(dt);
                }

                for (const auto &[id, entity] : m_entityMap) {
                    entity->update(dt);
                }
            }

            /**
             * @brief Shutdown the world system.
             * 
             * Clears all entities from the world. Should be called before
             * destroying the world system.
             */
            void shutdown() override { m_entityMap.clear(); }

            /**
             * @brief Set the physics system to use for simulation.
             * 
             * @param physicsSystem Shared pointer to the physics system
             */
            void setPhysics(std::shared_ptr<Physics::PhysicsSystem> physicsSystem) { m_physicsSystem = physicsSystem; }

            /**
             * @brief Add a new entity to the world.
             * 
             * Assigns a unique ID to the entity and registers it for updates.
             * If the entity has physics components, they are registered with
             * the physics system.
             * 
             * @param entity Shared pointer to the entity to add
             * @return The assigned unique entity ID
             */
            inline std::uint64_t addEntity(std::shared_ptr<IEntity> entity) { return insertEntity(std::move(entity)); }

            /**
             * @brief Remove all entities from the world.
             * 
             * Clears the entity map and resets the physics system. Useful
             * for clearing the scene before loading a new one.
             */
            inline void clearEntities() {
                m_entityMap.clear();
                if (m_physicsSystem) {
                    m_physicsSystem->reset();
                }
            }

            /**
             * @brief Remove a specific entity from the world.
             * 
             * @param entityID The ID of the entity to remove
             * 
             * @note This does not remove the physics body from the physics
             *       system, which may cause issues. Consider using clearEntities
             *       and re-adding entities instead.
             */
            inline void removeEntity(std::uint64_t entityID) { m_entityMap.erase(entityID); }

            /**
             * @brief Get a view of all entities in the world.
             * 
             * Creates a snapshot of all current entities. The returned view
             * is a copy and modifications to it won't affect the world.
             * 
             * @return WorldSystemView containing all entities
             */
            inline WorldSystemView view() const {
                WorldSystemView view;
                for (const auto &entity : m_entityMap) {
                    view.bodies.push_back(entity.second);
                }
                return view;
            }

            /**
             * @brief Update an entity's position.
             * 
             * @param entityID The ID of the entity to update
             * @param newPosition The new position to set
             */
            void updateEntityPosition(std::uint64_t entityID, const glm::vec3 &newPosition) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end()) {
                    it->second->setPosition(newPosition);
                }
            }

            /**
             * @brief Update an entity's rigid body mass.
             * 
             * @param entityID The ID of the entity to update
             * @param newMass The new mass value in kilograms
             */
            void updateEntityMass(std::uint64_t entityID, float newMass) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end()) {
                    auto rigidBody = it->second->getRigidBody();
                    if (rigidBody) {
                        rigidBody->setMass(newMass);
                    }
                }
            }

            /**
             * @brief Update an entity's scale.
             * 
             * @param entityID The ID of the entity to update
             * @param newScale The new scale vector
             */
            void updateEntityScale(std::uint64_t entityID, const glm::vec3 &newScale) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end()) {
                    it->second->setScale(newScale);
                }
            }

            /**
             * @brief Update whether an entity is affected by gravity.
             * 
             * @param entityID The ID of the entity to update
             * @param affected true to enable gravity, false to disable
             */
            void updateEntityImpactedByGravity(std::uint64_t entityID, bool affected) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end()) {
                    auto rigidBody = it->second->getRigidBody();
                    if (rigidBody) {
                        rigidBody->setUseGravity(affected);
                    }
                }
            }

            /**
             * @brief Update whether an entity is static (immovable).
             * 
             * @param entityID The ID of the entity to update
             * @param isStatic true to make static, false to make dynamic
             */
            void updateEntityStaticState(std::uint64_t entityID, bool isStatic) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end()) {
                    auto rigidBody = it->second->getRigidBody();
                    if (rigidBody) {
                        rigidBody->setStatic(isStatic);
                    }
                }
            }

            /**
             * @brief Reset the world to its initial state.
             * 
             * Restores all entities to their saved snapshot state. If no
             * snapshot exists, this has no effect.
             */
            void reset() { restoreInitialState(); }

            /**
             * @brief Save the current state of all entities.
             * 
             * Creates a snapshot of the world that can be restored later.
             * Used for simulation reset functionality.
             */
            void saveInitialState();
            
            /**
             * @brief Restore entities to the saved snapshot state.
             * 
             * Clears the current world and recreates all entities from the
             * saved snapshot. If no snapshot exists, just clears the world.
             */
            void restoreInitialState();

            /**
             * @brief Clear the saved state snapshot.
             * 
             * Frees memory used by the snapshot and resets the entity ID counter.
             */
            void clearInitialState() {
                m_initialState.clear();
                m_initialNextEntityID = 1;
            }

            /**
             * @brief Get the physics system.
             * @return Shared pointer to the physics system
             */
            std::shared_ptr<Arche::Physics::PhysicsSystem> getPhysicsSystem() const { return m_physicsSystem; }

          private:
            std::uint64_t m_nextEntityID{1};                                        ///< Next entity ID to assign
            std::unordered_map<std::uint64_t, std::shared_ptr<IEntity>> m_entityMap;///< Entity lookup table

            std::vector<EntitySnapshot> m_initialState;                             ///< Saved state snapshot
            std::uint64_t m_initialNextEntityID{1};                                 ///< Saved ID counter
            std::shared_ptr<Physics::PhysicsSystem> m_physicsSystem;               ///< Physics simulation
            std::shared_ptr<Arche::Core::LoggingService> m_logger;                 ///< Logger service

            /**
             * @brief Internal method to insert an entity with optional ID.
             * 
             * @param entity The entity to insert
             * @param forcedId Optional ID to use instead of auto-assigning
             * @return The entity's ID (assigned or forced)
             */
            std::uint64_t insertEntity(std::shared_ptr<IEntity> entity, std::optional<std::uint64_t> forcedId = std::nullopt);
        };

    } // namespace Scene
} // namespace Arche