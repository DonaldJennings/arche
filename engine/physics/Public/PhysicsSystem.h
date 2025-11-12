#pragma once
#include "ICollider.h"
#include "Rigidbody.h"
#include <ISubsystem.h>
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
            PhysicsSystem(std::shared_ptr<Arche::Core::LoggingService> logger) : m_logger(std::move(logger)) {}
            void initialise() override {};
            void shutdown() override {};
            void update(double deltaTime) override {
                // Integrate the physics world
                for (PhysicsBody &body : m_bodies) {
                    if (body.rb) {
                        body.rb->integrate(deltaTime, *body.position, m_gravity);
                    }
                }
            }

            void reset() { m_bodies.clear(); }

            glm::vec3 getGravity() const { return m_gravity; }

            void addBody(const PhysicsBody &body) { m_bodies.push_back(body); }

          private:
            std::vector<PhysicsBody> m_bodies;
            glm::vec3 m_gravity{0.0f, -9.81f, 0.0f};
            std::shared_ptr<Arche::Core::LoggingService> m_logger;
        };

    } // namespace Physics
} // namespace Arche