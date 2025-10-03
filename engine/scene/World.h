#ifndef ARCHE_ENGINE_WORLD_H
#define ARCHE_ENGINE_WORLD_H

#include <memory>
#include <vector>

#include "../core/Config.h"
#include "../math/SpatialTransform.h"
#include "../math/Vector3D.h"

namespace Arche {
    namespace Scene {

        struct Particle
        {
            Arche::Math::SpatialTransform transform;
            Arche::Math::Vector3D velocity;
            float mass;
        };

        struct BodyView {
            Arche::Math::SpatialTransform transform;
            Arche::Math::Vector3D velocity;
            float mass;
        };

        struct WorldView
        {
            std::vector<BodyView> bodies;
        };
        ;


        class World {
          public:
            static std::unique_ptr<World> Create(Arche::Core::WorldConfig const &config);

            World() = default;
            std::uint32_t createParticle(Arche::Math::SpatialTransform transform, float mass);
            void setGravity(Arche::Math::Vector3D const &gravity);
            void step();
            WorldView view() const;

            inline float stepDuration() const { return stepDuration_; }
            inline void setStepDuration(float duration) {
                stepDuration_ = duration;
            }

            double simulationTime() const { return simulationTime_; }
            Arche::Math::Vector3D gravity() const { return gravity_; };

          private:
            std::vector<Particle> particles_;
            Arche::Math::Vector3D gravity_;
            float stepDuration_ = 1.0f / 60.0f; // Default to 60 Hz
            double simulationTime_;
        };
    }
}
#endif