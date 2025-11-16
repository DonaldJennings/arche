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

        class WorldSystemView {
          public:
            std::vector<std::shared_ptr<IEntity>> bodies;
        };

        struct EntitySnapshot {
            std::uint64_t id{0};
            std::shared_ptr<IEntity> prototype;
        };

        class WorldSystem : public Core::ISubsystem {
          public:
            WorldSystem(std::shared_ptr<Arche::Core::LoggingService> logger,
                        std::shared_ptr<Arche::Core::TimingService> timing);

            void initialise() override {};

            void update(double dt) override {
                if (m_physicsSystem) {
                    m_physicsSystem->update(dt);
                }

                for (const auto &[id, entity] : m_entityMap) {
                    entity->update(dt);
                }
            }

            void shutdown() override { m_entityMap.clear(); }

            void setPhysics(std::shared_ptr<Physics::PhysicsSystem> physicsSystem) { m_physicsSystem = physicsSystem; }

            inline std::uint64_t addEntity(std::shared_ptr<IEntity> entity) { return insertEntity(std::move(entity)); }

            inline void clearEntities() {
                m_entityMap.clear();
                if (m_physicsSystem) {
                    m_physicsSystem->reset();
                }
            }

            inline void removeEntity(std::uint64_t entityID) { m_entityMap.erase(entityID); }

            inline WorldSystemView view() const {
                WorldSystemView view;
                for (const auto &entity : m_entityMap) {
                    view.bodies.push_back(entity.second);
                }
                return view;
            }

            void updateEntityPosition(std::uint64_t entityID, const glm::vec3 &newPosition) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end()) {
                    it->second->setPosition(newPosition);
                }
            }

            void updateEntityMass(std::uint64_t entityID, float newMass) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end()) {
                    auto rigidBody = it->second->getRigidBody();
                    if (rigidBody) {
                        rigidBody->setMass(newMass);
                    }
                }
            }

            void updateEntityScale(std::uint64_t entityID, const glm::vec3 &newScale) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end()) {
                    it->second->setScale(newScale);
                }
            }

            void updateEntityImpactedByGravity(std::uint64_t entityID, bool affected) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end()) {
                    auto rigidBody = it->second->getRigidBody();
                    if (rigidBody) {
                        rigidBody->setUseGravity(affected);
                    }
                }
            }

            void updateEntityStaticState(std::uint64_t entityID, bool isStatic) {
                auto it = m_entityMap.find(entityID);
                if (it != m_entityMap.end()) {
                    auto rigidBody = it->second->getRigidBody();
                    if (rigidBody) {
                        rigidBody->setStatic(isStatic);
                    }
                }
            }

            void reset() { restoreInitialState(); }

            void saveInitialState();
            void restoreInitialState();

            void clearInitialState() {
                m_initialState.clear();
                m_initialNextEntityID = 1;
            }

            std::shared_ptr<Arche::Physics::PhysicsSystem> getPhysicsSystem() const { return m_physicsSystem; }

          private:
            std::uint64_t m_nextEntityID{1};
            std::unordered_map<std::uint64_t, std::shared_ptr<IEntity>> m_entityMap;

            std::vector<EntitySnapshot> m_initialState;
            std::uint64_t m_initialNextEntityID{1};
            std::shared_ptr<Physics::PhysicsSystem> m_physicsSystem;
            std::shared_ptr<Arche::Core::LoggingService> m_logger;

            std::uint64_t insertEntity(std::shared_ptr<IEntity> entity, std::optional<std::uint64_t> forcedId = std::nullopt);
        };

    } // namespace Scene
} // namespace Arche