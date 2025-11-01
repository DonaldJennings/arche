#include "CameraController.h"
#include <cmath>

namespace Arche {
namespace GUI {

static constexpr float RAD_TO_DEG = 180.0f / 3.14159265358979323846f;
static constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

float CameraController::clamp(float v, float lo, float hi) noexcept {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float CameraController::clampPitch(float p) noexcept {
    // limit pitch to avoid gimbal-like flipping (just under +-89 degrees)
    constexpr float limit = 89.0f * DEG_TO_RAD;
    if (p < -limit) return -limit;
    if (p > limit) return limit;
    return p;
}

void CameraController::attachCamera(std::shared_ptr<Arche::Scene::Camera> cam) noexcept {
    camera = cam;
    if (camera) {
        updateCamera();
    }
}

void CameraController::detachCamera() noexcept {
    camera.reset();
}

void CameraController::rotate(float deltaYawRadians, float deltaPitchRadians) noexcept {
    yaw += deltaYawRadians * rotateSpeed;
    pitch += deltaPitchRadians * rotateSpeed;
    pitch = clampPitch(pitch);
}

void CameraController::zoom(float delta) noexcept {
    float factor = 1.0f + delta * zoomSpeed;
    if (factor <= 0.0f) factor = 0.001f;
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

void CameraController::updateCamera() noexcept {
    if (!camera) return;

    // Convert spherical coordinates (distance, yaw, pitch) to Cartesian coordinates around target
    float cx = distance * std::cos(pitch) * std::sin(yaw);
    float cy = distance * std::sin(pitch);
    float cz = distance * std::cos(pitch) * std::cos(yaw);

    Arche::Math::Vector3D pos(static_cast<double>(target.x() + cx), static_cast<double>(target.y() + cy), static_cast<double>(target.z() + cz));
    camera->SetPosition(pos);
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
