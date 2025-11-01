#ifndef ARCHE_ENGINE_WORLD_H
#define ARCHE_ENGINE_WORLD_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "../core/WorldConfig.h"
#include "../math/SpatialTransform.h"
#include "../math/Vector3D.h"

namespace Arche {
namespace Scene {

struct Particle {
    std::uint64_t id;
    Arche::Math::SpatialTransform transform;
    Arche::Math::Vector3D velocity;
    float mass;
    float scale;
};

struct BodyView {
    std::uint64_t id;
    Arche::Math::SpatialTransform transform;
    Arche::Math::Vector3D velocity;
    float mass;
    float scale;
};

struct WorldView {
    std::vector<BodyView> bodies;
};

/**
 * @brief The World class acts as a data/model container for particles and simulation state.
 *        All simulation logic and stepping is handled by the WorldSystem subsystem.
 */
class World {
  public:
    static std::shared_ptr<World> Create(const Arche::Core::WorldConfig& config);

    World();

    std::uint64_t createParticle(const Arche::Math::SpatialTransform& transform, float mass);
    void setGravity(const Arche::Math::Vector3D& gravity);
    void step(float dt);
    WorldView view() const;

    float stepDuration() const;
    void setStepDuration(float duration);

    void setObjectPosition(std::uint64_t objectID, const Arche::Math::Vector3D& position);
    void setObjectMass(std::uint64_t objectID, float mass);
    void setObjectScale(std::uint64_t objectID, float scale);

    double simulationTime() const;
    Arche::Math::Vector3D gravity() const;

  private:
    std::unordered_map<std::uint64_t, Particle> particles_;
    Arche::Math::Vector3D gravity_;
    float stepDuration_ = 1.0f / 60.0f; // Default to 60 Hz
    double simulationTime_ = 0.0;
    std::uint64_t nextParticleId_ = 1; // Start from 1 for clarity
};

struct WorldHolder {
    std::shared_ptr<World> world;
};

} // namespace Scene
} // namespace Arche
#endif