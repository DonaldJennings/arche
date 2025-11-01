#pragma once
#include "World.h"
#include <ISubsystem.h>
#include <JobPoolService.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <memory>
#include <Camera.h>

namespace Arche {
    namespace Scene {

        class WorldSystem : public Core::ISubsystem {
          public:
            WorldSystem(std::shared_ptr<World> world, std::shared_ptr<Arche::Core::LoggingService> logger, std::shared_ptr<Arche::Core::JobPoolService> jobPool,
                        std::shared_ptr<Arche::Core::TimingService> timing);

            void initialise() override;
            void update(float dt) override;
            void shutdown() override;

            void setWorld(std::shared_ptr<World> newWorld) { world_ = newWorld; }
            std::shared_ptr<World> getWorld() const;
            std::shared_ptr<Camera> getSceneCamera();

          private:
            std::shared_ptr<World> world_;
            std::shared_ptr<Arche::Core::LoggingService> logger_;
            std::shared_ptr<Arche::Core::JobPoolService> jobPool_;
            std::shared_ptr<Arche::Core::TimingService> timing_;
            std::shared_ptr<Camera> sceneCamera_;
        };

    } // namespace Scene
} // namespace Arche