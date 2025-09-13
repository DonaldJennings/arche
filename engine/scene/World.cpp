#include "World.h"

namespace Arche {
    namespace Scene {

        namespace {
            // Integration kernel: simple explicit Euler
            void integrateParticle(Particle &p,
                                   const Arche::Math::Vector3D &gravity,
                                   float dt) {
                if (p.mass > 0.0f) {
                    Arche::Math::Vector3D acceleration = gravity;
                    p.velocity = p.velocity + acceleration * dt;
                    p.transform.setPosition(
                        p.transform.getPosition() + p.velocity * dt);
                }
            }
        } // namespace

        std::unique_ptr<World>
        World::Create(const Arche::Core::WorldConfig &config) {
            auto world = std::make_unique<World>();
            world->gravity_ =
                Arche::Math::Vector3D(0.0, -9.81, 0.0); // Default gravity
            world->setStepDuration(config.stepDuration);
            return world;
        }

        std::uint32_t
        World::createParticle(Arche::Math::SpatialTransform transform,
                              float mass) {
            Particle p;
            p.transform = std::move(transform);
            p.velocity = Arche::Math::Vector3D(0.0, 0.0, 0.0);
            p.mass = mass;
            particles_.push_back(std::move(p));
            return static_cast<std::uint32_t>(particles_.size() - 1);
        }

        void World::setGravity(const Arche::Math::Vector3D &gravity) {
            gravity_ = gravity;
        }

        void World::step() {
            for (auto &p : particles_) {
                integrateParticle(p, gravity_, stepDuration_);
            }
        }

        WorldView World::view() const {
            WorldView v;
            for (const auto &p : particles_) {
                BodyView bv;
                bv.transform = p.transform;
                bv.velocity = p.velocity;
                bv.mass = p.mass;
                v.bodies.push_back(bv);
            }
            return v;
        }

    } // namespace Scene
} // namespace Arche