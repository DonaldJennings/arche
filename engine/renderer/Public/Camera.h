#pragma once
#include "World.h"
#include <ISubsystem.h>
#include <JobPoolService.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <array>
#include <memory>

namespace Arche {
    namespace Scene {

        class Camera {
          public:
            Camera() = default;
            void SetPosition(const Math::Vector3D &position);
            Math::Vector3D GetPosition();

            void SetPitch(double pitch);
            double GetPitch() const;

            void SetYaw(double yaw);
            double GetYaw() const;

            void setPitchYaw(double pitch, double yaw);

            void setPerspective(double fovY, double aspectRatio, double nearPlane, double farPlane);

            std::array<double, 16> GetViewMatrix();
            std::array<double, 16> GetProjectionMatrix();

          private:
            void RecalculateViewMatrix();
            void RecalculateProjectionMatrix();

            Arche::Math::Vector3D position_{0.0, 0.0, 0.0};
            double pitch_{0.0}; // in degrees
            double yaw_{0.0};   // in degrees

            double fovY_{60.0}; // in degrees
            double aspectRatio_{16.0 / 9.0};
            double nearPlane_{0.1};
            double farPlane_{1000.0};

            std::array<double, 16> viewMatrix_{};
            std::array<double, 16> projectionMatrix_{};
        };

    } // namespace Scene
} // namespace Arche