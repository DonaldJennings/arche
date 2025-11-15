#include "WorldSystem.h"

namespace Arche {
    namespace Scene {
        WorldSystem::WorldSystem(std::shared_ptr<Arche::Core::LoggingService> logger,
                                 std::shared_ptr<Arche::Core::TimingService> timing)
            : m_logger(std::move(logger)) {}

        void WorldSystem::saveInitialState() {
            m_initialState.clear();
            auto view = this->view();

            for (auto &e : view.bodies) {
                if (!e) {
                    continue;
                }

                m_initialState.push_back(e->clone());
            }
        }

        void WorldSystem::restoreInitialState() {
            for (auto &e : m_initialState) {
                if (!e) {
                    continue;
                }
                this->addEntity(e->clone());
            }
        }

        // std::shared_ptr<World> WorldSystem::getWorld() const { return m_world; }

    } // namespace Scene
} // namespace Arche
