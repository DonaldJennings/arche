#include "CameraController.h"
#include <cmath>

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
            // Move target in camera-local X/Y plane. Compute right and up vectors from yaw/pitch.
            // Right vector approximated by yaw-only; up is world-up (0,1,0).
            float rightX = std::cos(yaw);
            float rightZ = -std::sin(yaw);

            // apply pan in world units scaled by panSpeed and distance
            target.set_x(target.x() + (rightX * dx - rightZ * dy) * panSpeed * distance);
            target.set_y(target.y() + (dy * panSpeed * distance) * 0.5f); // small vertical component
            target.set_z(target.z() + (rightZ * dx + rightX * dy) * panSpeed * distance);
        }

        void CameraController::moveLocal(float forwardAmount, float rightAmount, float upAmount) noexcept {

            double cameraX{static_cast<double>(distance) * std::cos(static_cast<double>(pitch)) *
                           std::sin(static_cast<double>(yaw))};
            double cameraY{static_cast<double>(distance) * std::sin(static_cast<double>(pitch))};
            double cameraZ{static_cast<double>(distance) * std::cos(static_cast<double>(pitch)) *
                           std::cos(static_cast<double>(yaw))};

            // Camera position in World space
            double camPosX = target.x() + cameraX;
            double camPosY = target.y() + cameraY;
            double camPosZ = target.z() + cameraZ;

            // Forward vector (from camera to target)
            glm::dvec3 forwardDir = glm::normalize(
                glm::dvec3(static_cast<double>(target.x() - camPosX), static_cast<double>(target.y() - camPosY),
                                                              static_cast<double>(target.z() - camPosZ)));

            // Right vector
            glm::dvec3 worldUp = glm::dvec3(0.0, 1.0, 0.0);
            glm::dvec3 rightDir = glm::normalize(glm::cross(forwardDir, worldUp));

            // Recompute local up vector to ensure orthogonality
            glm::dvec3 upDir = glm::normalize(glm::cross(rightDir, forwardDir));

            // Apply movement to target
            double fMove = static_cast<double>(forwardAmount);
            double rMove = static_cast<double>(rightAmount);
            double uMove = static_cast<double>(upAmount);

            target.set_x(target.x() + forwardDir.x * fMove + rightDir.x * rMove + upDir.x * uMove);
            target.set_y(target.y() + forwardDir.y * fMove + rightDir.y * rMove + upDir.y * uMove);
            target.set_z(target.z() + forwardDir.z * fMove + rightDir.z * rMove + upDir.z * uMove);
        }

        void CameraController::updateCamera() noexcept {
            if (!camera)
                return;

            // Convert spherical coordinates (distance, yaw, pitch) to Cartesian coordinates around target
            float cx = distance * std::cos(pitch) * std::sin(yaw);
            float cy = distance * std::sin(pitch);
            float cz = distance * std::cos(pitch) * std::cos(yaw);

            Arche::Math::Vector3D pos(static_cast<double>(target.x() + cx), static_cast<double>(target.y() + cy),
                                      static_cast<double>(target.z() + cz));
            camera->SetPosition(glm::dvec3(pos.x(), pos.y(), pos.z()));
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
