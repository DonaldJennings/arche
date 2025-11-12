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

            inline std::uint64_t addEntity(std::shared_ptr<IEntity> entity) {

                uint64_t newEntityID = m_nextEntityID++;

                entity->setID(newEntityID);

                m_entityMap[newEntityID] = entity;

                if (m_physicsSystem) {
                    Physics::PhysicsBody body;
                    body.rb = entity->getRigidBody();
                    body.collider = entity->getCollider();
                    body.position = entity->getPositionPtr();

                    m_physicsSystem->addBody(body);
                }

                return newEntityID;
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

            void reset() {
                m_entityMap.clear();
                m_nextEntityID = 1;
                if (m_physicsSystem) {
                    m_physicsSystem->reset();
                }
            }

            std::optional<glm::vec3> getWorldGravity() const {
                if (m_physicsSystem) {
                    return m_physicsSystem->getGravity();
                }
                return std::nullopt;
            }

            std::shared_ptr<Arche::Physics::PhysicsSystem> getPhysicsSystem() const { return m_physicsSystem; }

          private:
            std::uint64_t m_nextEntityID{1};
            std::unordered_map<std::uint64_t, std::shared_ptr<IEntity>> m_entityMap;
            std::shared_ptr<Physics::PhysicsSystem> m_physicsSystem;
            std::shared_ptr<Arche::Core::LoggingService> m_logger;
        };

    } // namespace Scene
} // namespace Arche