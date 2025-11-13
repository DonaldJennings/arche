#include "CameraController.h"
#include <cmath>
#include <glm/glm.hpp>

namespace Arche {
    namespace GUI {

        static constexpr float RAD_TO_DEG = 180.0f / 3.14159265358979323846f;
        static constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

        float CameraController::clamp(float v, float lo, float hi) noexcept {
            if (v < lo)
                return lo;
            if (v > hi)
                return hi;
            return v;
        }

        float CameraController::clampPitch(float p) noexcept {
            // limit pitch to avoid gimbal-like flipping (just under +-89 degrees)
            constexpr float limit = 89.0f * DEG_TO_RAD;
            if (p < -limit)
                return -limit;
            if (p > limit)
                return limit;
            return p;
        }

        void CameraController::attachCamera(std::shared_ptr<Arche::Render::Camera> cam) noexcept {
            camera = cam;
            if (camera) {
                updateCamera();
            }
        }

        void CameraController::detachCamera() noexcept { camera.reset(); }

        void CameraController::rotate(float deltaYawRadians, float deltaPitchRadians) noexcept {
            yaw += deltaYawRadians * rotateSpeed;
            pitch += deltaPitchRadians * rotateSpeed;
            pitch = clampPitch(pitch);
        }

        void CameraController::zoom(float delta) noexcept {
            float factor = 1.0f + delta * zoomSpeed;
            if (factor <= 0.0f)
                factor = 0.001f;
            distance *= factor;
            distance = clamp(distance, minDistance, maxDistance);
        }

        void CameraController::pan(float dx, float dy) noexcept {
            // Right-handed: yaw=0 faces -Z, up=+Y, right=+X.
            glm::dvec3 t{ target.x, target.y, target.z };

            const double sy = std::sin(static_cast<double>(yaw));
            const double cy = std::cos(static_cast<double>(yaw));

            // Yaw-only right vector and world up
            const glm::dvec3 right{ cy, 0.0, sy };
            const glm::dvec3 up{ 0.0, 1.0, 0.0 };

            const double scale = static_cast<double>(panSpeed * distance);
            t += right * (static_cast<double>(dx) * scale);
            t += up    * (static_cast<double>(dy) * scale);

            target.x = t.x;
            target.y = t.y;
            target.z = t.z;
        }

        void CameraController::moveLocal(float forwardAmount, float rightAmount, float upAmount) noexcept {
            // Right-handed basis from yaw only (no pitch in strafing):
            // yaw=0 => forward = (0,0,-1), right = (1,0,0)
            glm::dvec3 t{ target.x, target.y, target.z };

            const double sy = std::sin(static_cast<double>(yaw));
            const double cy = std::cos(static_cast<double>(yaw));

            const glm::dvec3 forwardDir{ -sy, 0.0, -cy };
            const glm::dvec3 rightDir{    cy, 0.0,  sy };
            const glm::dvec3 upDir{ 0.0, 1.0, 0.0 };

            const double fMove = static_cast<double>(forwardAmount);
            const double rMove = static_cast<double>(rightAmount);
            const double uMove = static_cast<double>(upAmount);

            t += forwardDir * fMove + rightDir * rMove + upDir * uMove;

            // Sync back to controller target
            target.x = t.x;
            target.y = t.y;
            target.z = t.z;
        }

        void CameraController::updateCamera() noexcept {
            if (!camera)
                return;

            // Build forward vector from yaw/pitch (right-handed, yaw=0 -> -Z)
            const double sy = std::sin(static_cast<double>(yaw));
            const double cy = std::cos(static_cast<double>(yaw));
            const double sp = std::sin(static_cast<double>(pitch));
            const double cp = std::cos(static_cast<double>(pitch));

            const glm::dvec3 forward{
                cp * sy,
                sp,
                -cp * cy
            };

            const glm::dvec3 tgt{ static_cast<double>(target.x), static_cast<double>(target.y), static_cast<double>(target.z) };
            const glm::dvec3 eye = tgt - forward * static_cast<double>(distance);

            camera->SetPosition(eye);
            camera->SetPitch(static_cast<double>(pitch * RAD_TO_DEG));
            camera->SetYaw(static_cast<double>(yaw * RAD_TO_DEG));
        }

        void CameraController::updateOrientationOnly() noexcept {
            if (!camera)
                return;

            // Keep current camera position, only update orientation
            camera->SetPitch(static_cast<double>(pitch * RAD_TO_DEG));
            camera->SetYaw(static_cast<double>(yaw * RAD_TO_DEG));
        }

        void CameraController::setDistance(float d) noexcept { distance = clamp(d, minDistance, maxDistance); }
        float CameraController::getDistance() const noexcept { return distance; }

        void CameraController::setRotateSpeed(float s) noexcept { rotateSpeed = s; }
        void CameraController::setZoomSpeed(float s) noexcept { zoomSpeed = s; }

        float CameraController::getYawDegrees() const noexcept { return yaw * RAD_TO_DEG; }
        float CameraController::getPitchDegrees() const noexcept { return pitch * RAD_TO_DEG; }

    } // namespace GUI
} // namespace Arche
