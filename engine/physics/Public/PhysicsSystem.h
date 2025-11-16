#pragma once
#include "ICollider.h"
#include "Rigidbody.h"
#include <ISubsystem.h>
#include <LoggingService.h>
#include <GlobalSettings.h>
#include <memory>

namespace Arche {
    namespace Physics {

        /**
         * @brief Container for a complete physics body.
         * 
         * Associates a rigid body (mass, velocity, forces) with a collider
         * (shape) and position reference. This structure is used by the
         * physics system to track and update physical objects.
         */
        struct PhysicsBody {
            std::shared_ptr<RigidBody> rb;      ///< Rigid body dynamics component
            std::shared_ptr<ICollider> collider;///< Collision shape component
            glm::vec3 *position;                ///< Pointer to the entity's position
        };

        /**
         * @brief Main physics simulation subsystem.
         * 
         * PhysicsSystem manages the physics simulation for all dynamic objects
         * in the world. It performs numerical integration of rigid body dynamics,
         * applying gravity and other forces to update object positions and
         * velocities over time.
         * 
         * The system uses a simple Euler integration scheme and retrieves gravity
         * from the global settings. Bodies are integrated in parallel without
         * collision resolution (collision detection is not yet fully implemented).
         * 
         * @note This subsystem implements the ISubsystem interface for lifecycle
         *       management by the engine core.
         */
        class PhysicsSystem : public Arche::Core::ISubsystem {
          public:
            /**
             * @brief Construct the physics system.
             * 
             * @param logger Shared pointer to the logging service for diagnostics
             * @param settings Shared pointer to global settings (for gravity, etc.)
             */
            PhysicsSystem(std::shared_ptr<Arche::Core::LoggingService> logger, std::shared_ptr<Core::GlobalSettings> settings) 
                : m_logger(std::move(logger)), m_settings(std::move(settings)) {}

            /**
             * @brief Initialize the physics system.
             * 
             * Currently a no-op as the physics system doesn't require
             * initialization beyond construction.
             */
            void initialise() override {};
            
            /**
             * @brief Shutdown the physics system.
             * 
             * Releases any resources held by the physics system.
             */
            void shutdown() override {};
            
            /**
             * @brief Update the physics simulation.
             * 
             * Integrates all registered physics bodies forward in time by deltaTime.
             * Applies gravity from global settings and updates positions based on
             * velocities and accelerations.
             * 
             * @param deltaTime Time step in seconds (0 when simulation is paused)
             */
            void update(double deltaTime) override {
                // Get gravity directly from global settings
                const glm::vec3& gravity = m_settings->getWorldSettings().gravity;

                // Integrate the physics world
                for (PhysicsBody &body : m_bodies) {
                    if (body.rb) {
                        body.rb->integrate(deltaTime, *body.position, gravity);
                    }
                }
            }

            /**
             * @brief Remove all physics bodies from the simulation.
             * 
             * Clears the internal list of bodies. Typically called when
             * resetting or clearing the world.
             */
            inline void reset() { m_bodies.clear(); }

            /**
             * @brief Add a new physics body to the simulation.
             * 
             * Registers a body for physics integration. The body will be
             * updated on every call to update().
             * 
             * @param body The physics body to add (position pointer must remain valid)
             */
            inline void addBody(const PhysicsBody &body) { m_bodies.push_back(body); }

            /**
             * @brief Get all physics bodies in the simulation.
             * 
             * @return Vector copy of all registered physics bodies
             * 
             * @note Returns a copy, not a reference. Modifying the returned
             *       vector does not affect the internal state.
             */
            inline std::vector<PhysicsBody> getBodies() const { return m_bodies; }
          private:
            std::vector<PhysicsBody> m_bodies;                          ///< Active physics bodies
            std::shared_ptr<Arche::Core::LoggingService> m_logger;     ///< Logger for diagnostics
            std::shared_ptr<Core::GlobalSettings> m_settings;           ///< Global settings reference
        };

    } // namespace Physics
} // namespace Arche