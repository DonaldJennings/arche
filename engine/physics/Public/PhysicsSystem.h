#pragma once
#include "ICollider.h"
#include "Rigidbody.h"
#include <ISubsystem.h>
#include <LoggingService.h>
#include <GlobalSettings.h>
#include <memory>

namespace Arche {
    namespace Physics {

        struct PhysicsBody {
            std::shared_ptr<RigidBody> rb;
            std::shared_ptr<ICollider> collider;
            glm::vec3 *position;
        };

        class PhysicsSystem : public Arche::Core::ISubsystem {
          public:
            PhysicsSystem(std::shared_ptr<Arche::Core::LoggingService> logger, std::shared_ptr<Core::GlobalSettings> settings) 
                : m_logger(std::move(logger)), m_settings(std::move(settings)) {}

            void initialise() override {};
            void shutdown() override {};
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

            inline void reset() { m_bodies.clear(); }

            inline void addBody(const PhysicsBody &body) { m_bodies.push_back(body); }

            inline std::vector<PhysicsBody> getBodies() const { return m_bodies; }
          private:
            std::vector<PhysicsBody> m_bodies;
            std::shared_ptr<Arche::Core::LoggingService> m_logger;
            std::shared_ptr<Core::GlobalSettings> m_settings; // Store settings
        };

    } // namespace Physics
} // namespace Arche