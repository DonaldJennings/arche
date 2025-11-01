#include "WorldSystem.h"

namespace Arche {
    namespace Scene {
        WorldSystem::WorldSystem(std::shared_ptr<World> world, std::shared_ptr<Arche::Core::LoggingService> logger, std::shared_ptr<Arche::Core::JobPoolService> jobPool,
                                 std::shared_ptr<Arche::Core::TimingService> timing) : world_(std::move(world)), logger_(std::move(logger)), jobPool_(std::move(jobPool)), timing_(std::move(timing)) 
        {}
        void WorldSystem::initialise() {
    // Perform any world-specific initialization if needed
}

void WorldSystem::update(float dt) {
    if (world_) {
        // Optionally, use dt for more advanced stepping in the future
        world_->step(dt);
    }
}

void WorldSystem::shutdown() {
    // Cleanup resources if needed
    world_.reset();
}

std::shared_ptr<World> WorldSystem::getWorld() const {
    return world_;
}

} // namespace Scene
} // namespace Arche
