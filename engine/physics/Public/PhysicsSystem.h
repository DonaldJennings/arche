#pragma once
#include "ICollider.h"
#include "Rigidbody.h"
#include <ISubsystem.h>
#include <LoggingService.h>
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

            inline void reset() { m_bodies.clear(); }

            inline glm::vec3 getGravity() const { return m_gravity; }
            void setGravity(const glm::vec3 &gravity) { m_gravity = gravity; }

            inline void addBody(const PhysicsBody &body) { m_bodies.push_back(body); }

            inline std::vector<PhysicsBody> getBodies() const { return m_bodies; }
          private:
            std::vector<PhysicsBody> m_bodies;
            glm::vec3 m_gravity{0.0f, -9.81f, 0.0f};
            std::shared_ptr<Arche::Core::LoggingService> m_logger;
        };

    } // namespace Physics
} // namespace Arche