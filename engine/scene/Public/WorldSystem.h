#pragma once
#include "IEntity.h"
#include <ISubsystem.h>
#include <LoggingService.h>
#include <PhysicsSystem.h>
#include <RenderScene.h>
#include <TimingService.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <optional>

namespace Arche {
    namespace Scene {

        /**
         * @brief View snapshot of the world state.
         */
        class WorldSystemView {
          public:
            std::vector<std::shared_ptr<IEntity>> bodies;
        };

        /**
         * @brief Snapshot of an entity's state for save/restore.
         */
        struct EntitySnapshot {
            std::uint64_t id{0};
            std::shared_ptr<IEntity> prototype;
        };

        /**
         * @brief Main scene management subsystem.
         *
         * Manages entity lifecycle and coordinates with PhysicsSystem.
         * After each physics step, positions are synced from the physics SoA
         * back to entities. Each frame, WorldSystem builds a RenderScene for
         * the renderer without exposing WorldSystem itself to the render layer.
         */
        class WorldSystem : public Core::ISubsystem {
          public:
            WorldSystem(std::shared_ptr<Arche::Core::LoggingService> logger,
                        std::shared_ptr<Arche::Core::TimingService> timing);

            void initialise() override {}

            /**
             * @brief Tick physics, sync positions back to entities, then update entities.
             * @param dt Frame delta time in seconds
             */
            void update(double dt) override {
                if (m_physicsSystem) {
                    m_physicsSystem->update(dt);

                    // Sync physics-owned positions back to entities
                    const auto &ids = m_physicsSystem->getEntityIds();
                    const auto &pos = m_physicsSystem->getPositions();
                    for (std::size_t i = 0; i < ids.size(); ++i) {
                        auto it = m_entityMap.find(ids[i]);
                        if (it != m_entityMap.end())
                            it->second->setPosition(pos[i]);
                    }
                }

                for (const auto &[id, entity] : m_entityMap)
                    entity->update(dt);
            }

            void shutdown() override { m_entityMap.clear(); }

            void setPhysics(std::shared_ptr<Physics::PhysicsSystem> physicsSystem) {
                m_physicsSystem = physicsSystem;
            }

            inline std::uint64_t addEntity(std::shared_ptr<IEntity> entity) {
                return insertEntity(std::move(entity));
            }

            inline void clearEntities() {
                m_entityMap.clear();
                if (m_physicsSystem)
                    m_physicsSystem->reset();
            }

            /**
             * @brief Remove an entity and its physics body.
             * @param entityID ID of the entity to remove
             */
            inline void removeEntity(std::uint64_t entityID) {
                m_entityMap.erase(entityID);
                if (m_physicsSystem)
                    m_physicsSystem->removeBody(entityID);
            }

            inline WorldSystemView view() const {
                WorldSystemView v;
                for (const auto &e : m_entityMap)
                    v.bodies.push_back(e.second);
                return v;
            }

            /**
             * @brief Build a RenderScene from the current entity map.
             *
             * Called by EngineCore each frame before rendering. The renderer
             * receives only a RenderScene, so RenderingSystem has no dependency
             * on WorldSystem.
             */
            Render::RenderScene buildRenderScene() const {
                Render::RenderScene scene;
                scene.opaqueObjects.reserve(m_entityMap.size());
                for (const auto &[id, entity] : m_entityMap) {
                    Render::RenderObject obj;
                    obj.meshId     = std::string(entity->getMeshId());
                    obj.materialId = std::string(entity->getMaterialId());
                    obj.transform  = glm::scale(
                        glm::translate(glm::mat4(1.0f), entity->getPosition()),
                        entity->getScale());
                    scene.opaqueObjects.push_back(std::move(obj));
                }
                return scene;
            }

            // --- Property update helpers (used by editor panels) ---

            void updateEntityPosition(std::uint64_t entityID, const glm::vec3 &newPosition) {
                auto it = m_entityMap.find(entityID);
                if (it == m_entityMap.end()) return;
                it->second->setPosition(newPosition);
                if (m_physicsSystem)
                    m_physicsSystem->setPosition(entityID, newPosition);
            }

            void updateEntityMass(std::uint64_t entityID, float newMass) {
                auto it = m_entityMap.find(entityID);
                if (it == m_entityMap.end()) return;
                if (auto rb = it->second->getRigidBody())
                    rb->setMass(newMass);
                if (m_physicsSystem)
                    m_physicsSystem->setMass(entityID, newMass);
            }

            void updateEntityScale(std::uint64_t entityID, const glm::vec3 &newScale) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end())
                    it->second->setScale(newScale);
            }

            void updateEntityImpactedByGravity(std::uint64_t entityID, bool affected) {
                auto it = m_entityMap.find(entityID);
                if (it == m_entityMap.end()) return;
                if (auto rb = it->second->getRigidBody())
                    rb->setUseGravity(affected);
                if (m_physicsSystem)
                    m_physicsSystem->setUseGravity(entityID, affected);
            }

            void updateEntityStaticState(std::uint64_t entityID, bool isStatic) {
                auto it = m_entityMap.find(entityID);
                if (it == m_entityMap.end()) return;
                if (auto rb = it->second->getRigidBody())
                    rb->setStatic(isStatic);
                if (m_physicsSystem)
                    m_physicsSystem->setIsStatic(entityID, isStatic);
            }

            void reset() { restoreInitialState(); }
            void saveInitialState();
            void restoreInitialState();

            void clearInitialState() {
                m_initialState.clear();
                m_initialNextEntityID = 1;
            }

            std::shared_ptr<Arche::Physics::PhysicsSystem> getPhysicsSystem() const {
                return m_physicsSystem;
            }

          private:
            std::uint64_t m_nextEntityID{1};
            std::unordered_map<std::uint64_t, std::shared_ptr<IEntity>> m_entityMap;

            std::vector<EntitySnapshot> m_initialState;
            std::uint64_t m_initialNextEntityID{1};
            std::shared_ptr<Physics::PhysicsSystem> m_physicsSystem;
            std::shared_ptr<Arche::Core::LoggingService> m_logger;

            std::uint64_t insertEntity(std::shared_ptr<IEntity> entity,
                                       std::optional<std::uint64_t> forcedId = std::nullopt);
        };

    } // namespace Scene
} // namespace Arche
