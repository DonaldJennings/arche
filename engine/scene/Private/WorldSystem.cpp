#include "WorldSystem.h"

#include <algorithm>

namespace Arche {
    namespace Scene {
        WorldSystem::WorldSystem(std::shared_ptr<Arche::Core::LoggingService> logger,
                                 std::shared_ptr<Arche::Core::TimingService> timing)
            : m_logger(std::move(logger)) {}

        std::uint64_t WorldSystem::insertEntity(std::shared_ptr<IEntity> entity, std::optional<std::uint64_t> forcedId) {
            if (!entity) {
                return 0;
            }

            const std::uint64_t assignedId = forcedId.has_value() ? *forcedId : m_nextEntityID++;
            m_nextEntityID = std::max<std::uint64_t>(m_nextEntityID, assignedId + 1);

            entity->setID(assignedId);
            m_entityMap[assignedId] = entity;

            if (m_physicsSystem) {
                Physics::PhysicsBody body;
                body.rb = entity->getRigidBody();
                body.collider = entity->getCollider();
                body.position = entity->getPositionPtr();

                m_physicsSystem->addBody(body);
            }

            return assignedId;
        }

        void WorldSystem::saveInitialState() {
            m_initialState.clear();
            auto view = this->view();

            for (auto &e : view.bodies) {
                if (!e) {
                    continue;
                }

                EntitySnapshot snapshot{};
                snapshot.id = e->getID();
                snapshot.prototype = e->clone();
                m_initialState.push_back(std::move(snapshot));
            }

            m_initialNextEntityID = m_nextEntityID;
        }

        void WorldSystem::restoreInitialState() {
            const std::uint64_t initialNextId = m_initialNextEntityID;

            clearEntities();
            m_nextEntityID = initialNextId;

            for (auto &snapshot : m_initialState) {
                if (!snapshot.prototype) {
                    continue;
                }
                insertEntity(snapshot.prototype->clone(), snapshot.id);
            }

            m_nextEntityID = std::max(m_initialNextEntityID, m_nextEntityID);
        }

    } // namespace Scene
} // namespace Arche
