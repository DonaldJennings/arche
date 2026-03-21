#pragma once
#include "ICollider.h"
#include "Rigidbody.h"
#include <ISubsystem.h>
#include <LoggingService.h>
#include <GlobalSettings.h>
#include <memory>
#include <vector>

namespace Arche {
    namespace Physics {

        /**
         * @brief Input descriptor for registering a body with PhysicsSystem.
         *
         * Used only as an argument to addBody(). Not stored internally;
         * data is unpacked into the SoA arrays.
         */
        struct PhysicsBody {
            std::uint64_t entityId{0};           ///< Owning entity ID
            std::shared_ptr<RigidBody> rb;        ///< Initial physics properties
            std::shared_ptr<ICollider> collider;  ///< Collision shape
            glm::vec3 position{0.0f};             ///< Initial position (value, not pointer)
        };

        /**
         * @brief Main physics simulation subsystem.
         *
         * Stores all physics state as Structure-of-Arrays (SoA) for cache
         * efficiency and direct mapping to future GPU storage buffers.
         * Positions are owned by PhysicsSystem and synced back to entities
         * by WorldSystem after each update.
         */
        class PhysicsSystem : public Arche::Core::ISubsystem {
          public:
            PhysicsSystem(std::shared_ptr<Arche::Core::LoggingService> logger,
                          std::shared_ptr<Core::GlobalSettings> settings)
                : m_logger(std::move(logger)), m_settings(std::move(settings)) {}

            void initialise() override {}
            void shutdown() override {}

            /**
             * @brief Integrate all dynamic bodies forward by deltaTime.
             *
             * Applies gravity, integrates velocity → position, resets acceleration.
             * Static bodies are skipped.
             *
             * @param deltaTime Time step in seconds
             */
            void update(double deltaTime) override {
                const glm::vec3 gravity = m_settings->getWorldSettings().gravity;
                const float dt = static_cast<float>(deltaTime);

                #pragma omp parallel for schedule(static) // parallelise over bodies
                for (int i = 0; i < m_entityIds.size(); ++i) {
                    if (m_isStatic[i])
                        continue;

                    if (m_useGravity[i])
                        m_accelerations[i] += gravity;

                    m_velocities[i]    += m_accelerations[i] * dt;
                    m_positions[i]     += m_velocities[i]    * dt;
                    m_accelerations[i]  = glm::vec3(0.0f);
                }
            }

            /**
             * @brief Register a new body. Data is unpacked into the SoA arrays.
             * @param body Input descriptor with entity ID, RigidBody, and initial position.
             */
            void addBody(const PhysicsBody &body) {
                m_entityIds.push_back(body.entityId);
                m_positions.push_back(body.position);
                m_velocities.push_back(body.rb ? body.rb->getVelocity()     : glm::vec3(0.0f));
                m_accelerations.push_back(body.rb ? body.rb->getAcceleration() : glm::vec3(0.0f));
                m_masses.push_back(body.rb ? body.rb->getMass()       : 1.0f);
                m_isStatic.push_back(body.rb ? body.rb->isStatic()    : true);
                m_useGravity.push_back(body.rb ? body.rb->isUsingGravity() : false);
                m_colliders.push_back(body.collider);
            }

            /**
             * @brief Remove the body associated with entityId (swap-and-pop, O(n)).
             * @param entityId ID of the entity whose body should be removed.
             */
            void removeBody(std::uint64_t entityId) {
                for (std::size_t i = 0; i < m_entityIds.size(); ++i) {
                    if (m_entityIds[i] != entityId)
                        continue;

                    const std::size_t last = m_entityIds.size() - 1;
                    m_entityIds[i]     = m_entityIds[last];
                    m_positions[i]     = m_positions[last];
                    m_velocities[i]    = m_velocities[last];
                    m_accelerations[i] = m_accelerations[last];
                    m_masses[i]        = m_masses[last];
                    m_isStatic[i]      = m_isStatic[last];
                    m_useGravity[i]    = m_useGravity[last];
                    m_colliders[i]     = m_colliders[last];

                    m_entityIds.pop_back();
                    m_positions.pop_back();
                    m_velocities.pop_back();
                    m_accelerations.pop_back();
                    m_masses.pop_back();
                    m_isStatic.pop_back();
                    m_useGravity.pop_back();
                    m_colliders.pop_back();
                    return;
                }
            }

            /// Remove all bodies.
            void reset() {
                m_entityIds.clear();
                m_positions.clear();
                m_velocities.clear();
                m_accelerations.clear();
                m_masses.clear();
                m_isStatic.clear();
                m_useGravity.clear();
                m_colliders.clear();
            }

            // --- SoA read accessors (used by WorldSystem to sync positions) ---
            const std::vector<std::uint64_t>&            getEntityIds()    const { return m_entityIds; }
            const std::vector<glm::vec3>&                getPositions()    const { return m_positions; }

            // --- Property setters (editor / WorldSystem property updates) ---
            void setPosition(std::uint64_t entityId, const glm::vec3 &pos) {
                if (auto i = findIndex(entityId); i != npos) m_positions[i] = pos;
            }
            void setMass(std::uint64_t entityId, float mass) {
                if (auto i = findIndex(entityId); i != npos) m_masses[i] = mass;
            }
            void setUseGravity(std::uint64_t entityId, bool useGravity) {
                if (auto i = findIndex(entityId); i != npos) m_useGravity[i] = useGravity;
            }
            void setIsStatic(std::uint64_t entityId, bool isStatic) {
                if (auto i = findIndex(entityId); i != npos) m_isStatic[i] = isStatic;
            }

          private:
            static constexpr std::size_t npos = static_cast<std::size_t>(-1);

            std::size_t findIndex(std::uint64_t entityId) const {
                for (std::size_t i = 0; i < m_entityIds.size(); ++i)
                    if (m_entityIds[i] == entityId) return i;
                return npos;
            }

            // SoA physics data
            std::vector<std::uint64_t>            m_entityIds;
            std::vector<glm::vec3>                m_positions;
            std::vector<glm::vec3>                m_velocities;
            std::vector<glm::vec3>                m_accelerations;
            std::vector<float>                    m_masses;
            std::vector<bool>                     m_isStatic;
            std::vector<bool>                     m_useGravity;
            std::vector<std::shared_ptr<ICollider>> m_colliders;

            std::shared_ptr<Arche::Core::LoggingService> m_logger;
            std::shared_ptr<Core::GlobalSettings>        m_settings;
        };

    } // namespace Physics
} // namespace Arche
