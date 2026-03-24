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

        void CameraController::rotate(float yawDeltaDeg, float pitchDeltaDeg) noexcept {
            yaw   += yawDeltaDeg;
            pitch += pitchDeltaDeg;
            // keep yaw bounded (degrees)
            if (yaw > 180.0f)  yaw -= 360.0f;
            if (yaw < -180.0f) yaw += 360.0f;
            // clamp pitch (degrees)
            pitch = glm::clamp(pitch, -89.0f, 89.0f);
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
            const double yawR = glm::radians(static_cast<double>(yaw)); // convert to radians
            glm::dvec3 t{ target.x, target.y, target.z };

            const glm::dvec3 right{ std::cos(yawR), 0.0, -std::sin(yawR) };
            const glm::dvec3 up{ 0.0, 1.0, 0.0 };

            const double scale = static_cast<double>(panSpeed * distance);
            t += right * (static_cast<double>(dx) * scale);
            t += up    * (static_cast<double>(dy) * scale);

            target = glm::vec3(t);
        }

        void CameraController::moveLocal(float forward, float right, float up) {
            if (!camera) return;

            // radians
            const double yawR   = glm::radians(yaw);
            const double pitchR = glm::radians(pitch);

            // camera basis from yaw/pitch
            const double cy = std::cos(yawR),  sy = std::sin(yawR);
            const double cp = std::cos(pitchR), sp = std::sin(pitchR);

            // -Z forward convention (OpenGL-style)
            glm::dvec3 f = glm::normalize(glm::dvec3(-sy * cp, sp, -cy * cp));
            glm::dvec3 r = glm::normalize(glm::cross(f, glm::dvec3(0.0, 1.0, 0.0)));
            glm::dvec3 u = glm::normalize(glm::cross(r, f));

            // accumulate in world space
            position += f * static_cast<double>(forward)
                     +  r * static_cast<double>(right)
                     +  u * static_cast<double>(up);

            camera->SetPosition(position);
        }

        void CameraController::updateCamera() noexcept {
            const double yawR   = glm::radians(yaw);
            const double pitchR = glm::radians(pitch);
            const double cy = std::cos(yawR),  sy = std::sin(yawR);
            const double cp = std::cos(pitchR), sp = std::sin(pitchR);
            glm::dvec3 f = glm::normalize(glm::dvec3(sy * cp, -sp, -cy * cp));
            glm::dvec3 up(0.0, 1.0, 0.0);

            // If your Camera supports LookAt:
            // camera->setPerspective(position_, position_ + f, up);

            // Or if it stores yaw/pitch:
            camera->SetPosition(position);
            camera->SetYaw(yaw);
            camera->SetPitch(pitch);
        }

        void CameraController::updateOrientationOnly() noexcept {
            if (!camera) return;
            // yaw/pitch are already in degrees; set directly
            camera->SetYaw(yaw);
            camera->SetPitch(pitch);
        }

        void CameraController::setDistance(float d) noexcept { distance = clamp(d, minDistance, maxDistance); }
        float CameraController::getDistance() const noexcept { return distance; }

        void CameraController::setRotateSpeed(float s) noexcept { rotateSpeed = s; }
        void CameraController::setZoomSpeed(float s) noexcept { zoomSpeed = s; }

        float CameraController::getYawDegrees() const noexcept   { return yaw; }
        float CameraController::getPitchDegrees() const noexcept { return pitch; }

    } // namespace GUI
} // namespace Arche
