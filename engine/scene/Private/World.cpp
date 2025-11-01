#include "World.h"

namespace Arche {
namespace Scene {

std::shared_ptr<World> World::Create(const Arche::Core::WorldConfig& config) {
    auto world = std::make_shared<World>();
    world->gravity_ = config.gravity;
    world->stepDuration_ = config.stepDuration;
    world->simulationTime_ = 0.0;
    return world;
}

World::World()
    : gravity_(0.0f, -9.81f, 0.0f), stepDuration_(1.0f / 60.0f), simulationTime_(0.0), nextParticleId_(1) {}

std::uint64_t World::createParticle(const Arche::Math::SpatialTransform& transform, float mass) {
    Particle p;
    p.id = nextParticleId_++;
    p.transform = transform;
    p.velocity = Arche::Math::Vector3D(0.0f, 0.0f, 0.0f);
    p.mass = mass;
    p.scale = 1.0f;
    particles_[p.id] = p;
    return p.id;
}

void World::setGravity(const Arche::Math::Vector3D& gravity) {
    gravity_ = gravity;
}

void World::step(float dt) {
    // TODO: Might need to rethink this once we have more complex physics
    // Simple Euler integration for demonstration
    for (auto& [id, particle] : particles_) {
        // F = m * a; a = gravity
        Arche::Math::Vector3D acceleration = gravity_;
        particle.velocity = particle.velocity + acceleration * dt;
        Arche::Math::Vector3D newPos = particle.transform.getPosition() + particle.velocity * dt;
        particle.transform.setPosition(newPos);
    }
    simulationTime_ += dt;
}

WorldView World::view() const {
    WorldView v;
    for (const auto& [id, particle] : particles_) {
        v.bodies.push_back(BodyView{
            particle.id,
            particle.transform,
            particle.velocity,
            particle.mass,
            particle.scale
        });
    }
    return v;
}

float World::stepDuration() const {
    return stepDuration_;
}

void World::setStepDuration(float duration) {
    stepDuration_ = duration;
}

void World::setObjectPosition(std::uint64_t objectID, const Arche::Math::Vector3D& position) {
    auto it = particles_.find(objectID);
    if (it != particles_.end()) {
        it->second.transform.setPosition(position);
    }
}

void World::setObjectMass(std::uint64_t objectID, float mass) {
    auto it = particles_.find(objectID);
    if (it != particles_.end()) {
        it->second.mass = mass;
    }
}

void World::setObjectScale(std::uint64_t objectID, float scale) {
    auto it = particles_.find(objectID);
    if (it != particles_.end()) {
        it->second.transform.setScale(Arche::Math::Vector3D(scale, scale, scale));
    }
}

double World::simulationTime() const {
    return simulationTime_;
}

Arche::Math::Vector3D World::gravity() const {
    return gravity_;
}

} // namespace Scene
} // namespace Arche