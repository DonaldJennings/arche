#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <Camera.h>

namespace Arche {
namespace GUI {


/**
 * Minimal CameraController: only the essentials required to control
 * the camera's position/orientation in 3D for the first iteration.
 *
 * Responsibilities:
 *  - hold a reference to the engine Camera
 *  - provide simple rotate/zoom operations that update the Camera
 *  - expose a tiny configuration surface (speeds, distance)
 *
 * All non-essential features (panning, input state tracking, UI glue,
 * target bookkeeping, optional helpers) have been removed.
 */
class CameraController {
public:
    CameraController() = default;

    // Attach / detach the engine camera the controller drives.
    void attachCamera(std::shared_ptr<Arche::Render::Camera> cam) noexcept;
    void detachCamera() noexcept;

    // Direct control operations (minimal):
    // - rotate: adjust yaw/pitch (radians)
    // - zoom: adjust distance (multiplicative)
    void rotate(float deltaYawRadians, float deltaPitchRadians) noexcept;
    void zoom(float delta) noexcept;

    // panning: translate camera target in world-space X/Y plane
    void pan(float dx, float dy) noexcept;

    // Move camera in local space (forward/right/up) by given amounts.
    void moveLocal(float forwardAmount, float rightAmount, float upAmount);

    // Immediately apply controller state to attached Camera.
    // - updateCamera: orbit-style update (position changes based on yaw/pitch/distance/target)
    // - updateOrientationOnly: only updates yaw/pitch on the Camera, keeping its position unchanged
    void updateCamera() noexcept;
    void updateOrientationOnly() noexcept;

    // Small configuration
    void setDistance(float d) noexcept;
    float getDistance() const noexcept;

    void setRotateSpeed(float s) noexcept;
    void setZoomSpeed(float s) noexcept;

    float getYawDegrees() const noexcept;
    float getPitchDegrees() const noexcept;

    void reset() noexcept {
        distance = 5.0f;
        yaw = 0.0f;
        pitch = 0.0f;
        target = glm::vec3{0.0, 0.0, 0.0};
    }

private:
    static float clamp(float v, float lo, float hi) noexcept;
    static float clampPitch(float p) noexcept;

private:
    // Minimal state: spherical coordinates (around target)
    float distance{5.0f};   // radius from target
    float yaw{0.0f};        // radians
    float pitch{0.0f};      // radians
    glm::vec3 position{0.0, 0.0, 5.0}; // cached position (world space)

    // target in world space (panning)
    glm::vec3 target{0.0, 0.0, 0.0};

    // Sensitivity / limits
    float rotateSpeed{0.005f};
    float zoomSpeed{0.1f};
    float panSpeed{0.01f};
    float minDistance{0.1f};
    float maxDistance{1000.0f};

    // Attached engine camera (owned externally)
    std::shared_ptr<Arche::Render::Camera> camera;
};

} // namespace GUI
} // namespace Arche