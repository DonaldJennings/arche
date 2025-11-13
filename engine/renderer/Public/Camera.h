#pragma once
#include <array>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace Arche {
    namespace Render {

        class Camera {
          public:
            Camera() = default;

            // Position using GLM double-precision vector
            void SetPosition(const glm::dvec3 &position);
            glm::dvec3 GetPosition() const;

            void SetPitch(double pitch);
            double GetPitch() const;

            void SetYaw(double yaw);
            double GetYaw() const;

            void setPitchYaw(double pitch, double yaw);

            void setPerspective(double fovY, double aspectRatio, double nearPlane, double farPlane);

            // Returns a world-space ray direction (normalized) for the given screen coords.
            // Coordinates: screenX/screenY in pixels, viewportWidth/viewportHeight in pixels.
            glm::dvec3 screenToWorldRay(float screenX, float screenY, float viewportWidth, float viewportHeight);

            // Keep legacy layout: column-major 4x4 matrix as 16 double values
            glm::dmat4 GetViewMatrix() const ;
            glm::dmat4 GetProjectionMatrix() const;

          private:
            void RecalculateViewMatrix();
            void RecalculateProjectionMatrix();

            // Use GLM double-precision vector for position
            glm::dvec3 position_{0.0, 0.0, 0.0};
            double pitch_{0.0}; // in degrees
            double yaw_{0.0};   // in degrees

            double fovY_{60.0}; // in degrees
            double aspectRatio_{16.0 / 9.0};
            double nearPlane_{0.1};
            double farPlane_{1000.0};

            // Keep existing public API returning std::array<double,16>
            glm::dmat4 viewMatrix_{};
            glm::dmat4 projectionMatrix_{};
        };

    } // namespace Scene
} // namespace Arche