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

        void CameraController::attachCamera(std::shared_ptr<Arche::Scene::Camera> cam) noexcept {
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
            // Work in glm space and sync back to target
            glm::dvec3 t{ target.x, target.y, target.z };

            // Right vector approximated by yaw-only
            double rightX = std::cos(static_cast<double>(yaw));
            double rightZ = -std::sin(static_cast<double>(yaw));

            // Apply pan in world units scaled by panSpeed and distance
            double scale = static_cast<double>(panSpeed * distance);
            t.x += (rightX * static_cast<double>(dx) - rightZ * static_cast<double>(dy)) * scale;
            t.y += (static_cast<double>(dy) * scale) * 0.5; // small vertical component
            t.z += (rightZ * static_cast<double>(dx) + rightX * static_cast<double>(dy)) * scale;

            // Sync back to controller target
            target.x = t.x;
            target.y = t.y;
            target.z = t.z;
        }

        void CameraController::moveLocal(float forwardAmount, float rightAmount, float upAmount) noexcept {
            // Convert controller target to glm, compute camera position in world space
            glm::dvec3 t{ target.x, target.y, target.z };

            double camX = static_cast<double>(distance) * std::cos(static_cast<double>(pitch)) * std::sin(static_cast<double>(yaw));
            double camY = static_cast<double>(distance) * std::sin(static_cast<double>(pitch));
            double camZ = static_cast<double>(distance) * std::cos(static_cast<double>(pitch)) * std::cos(static_cast<double>(yaw));
            glm::dvec3 camPos = t + glm::dvec3(camX, camY, camZ);

            // Basis vectors
            glm::dvec3 forwardDir = glm::normalize(t - camPos);
            glm::dvec3 worldUp{0.0, 1.0, 0.0};
            glm::dvec3 rightDir = glm::normalize(glm::cross(forwardDir, worldUp));
            glm::dvec3 upDir = glm::normalize(glm::cross(rightDir, forwardDir));

            // Apply movement in local space
            double fMove = static_cast<double>(forwardAmount);
            double rMove = static_cast<double>(rightAmount);
            double uMove = static_cast<double>(upAmount);

            t += forwardDir * fMove + rightDir * rMove + upDir * uMove;

            // Sync back to controller target
            target.x = t.x;
            target.y = t.y;
            target.z = t.z;
        }

        void CameraController::updateCamera() noexcept {
            if (!camera)
                return;

            // Convert spherical coordinates (distance, yaw, pitch) to Cartesian coordinates around target
            double cx = static_cast<double>(distance) * std::cos(static_cast<double>(pitch)) * std::sin(static_cast<double>(yaw));
            double cy = static_cast<double>(distance) * std::sin(static_cast<double>(pitch));
            double cz = static_cast<double>(distance) * std::cos(static_cast<double>(pitch)) * std::cos(static_cast<double>(yaw));

            glm::dvec3 camPos{
                static_cast<double>(target.x) + cx,
                static_cast<double>(target.y) + cy,
                static_cast<double>(target.z) + cz
            };

            camera->SetPosition(camPos);
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
