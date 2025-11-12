#include "WorldSystem.h"

namespace Arche {
    namespace Scene {
        WorldSystem::WorldSystem(std::shared_ptr<Arche::Core::LoggingService> logger, std::shared_ptr<Arche::Core::TimingService> timing)
            : m_logger(std::move(logger)) {}

        // std::shared_ptr<World> WorldSystem::getWorld() const { return m_world; }

    } // namespace Scene
} // namespace Arche
