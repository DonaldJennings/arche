#pragma once
#include "Camera.h"
#include <ISubsystem.h>
#include <cmath>
#include <memory>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Arche {
namespace Render {

// Helper using GLM utilities
static double DegToRad(double d) { return glm::radians(d); }

void Camera::SetPosition(const glm::dvec3 &position) {
    position_ = position;
    RecalculateViewMatrix();
}

glm::dvec3 Camera::GetPosition() const {
    return position_;
}

void Camera::SetPitch(double pitch) {
    pitch_ = pitch;
    RecalculateViewMatrix();
}

double Camera::GetPitch() const {
    return pitch_;
}

void Camera::SetYaw(double yaw) {
    yaw_ = yaw;
    RecalculateViewMatrix();
}

double Camera::GetYaw() const {
    return yaw_;
}

void Camera::setPitchYaw(double pitch, double yaw) {
    pitch_ = pitch;
    yaw_ = yaw;
    RecalculateViewMatrix();
}

void Camera::setPerspective(double fovY, double aspectRatio, double nearPlane, double farPlane) {
    fovY_ = fovY;
    aspectRatio_ = aspectRatio;
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;
    RecalculateProjectionMatrix();
}

glm::dmat4 Camera::GetViewMatrix() const {
    return viewMatrix_;
}

glm::dmat4 Camera::GetProjectionMatrix() const {
    return projectionMatrix_;
}

void Camera::RecalculateViewMatrix() {
    // Compute forward vector from pitch/yaw (degrees) - yaw=0 looks down -Z
    const double pitchRad = DegToRad(pitch_);
    const double yawRad = DegToRad(yaw_);

    const double fx = std::cos(pitchRad) * std::sin(yawRad);
    const double fy = std::sin(pitchRad);
    const double fz = -std::cos(pitchRad) * std::cos(yawRad);
    glm::dvec3 forward = glm::normalize(glm::dvec3(fx, fy, fz));

    glm::dvec3 worldUp = glm::dvec3(0.0, 1.0, 0.0);
    glm::dvec3 right = glm::normalize(glm::cross(forward, worldUp));
    glm::dvec3 up = glm::normalize(glm::cross(right, forward));

    glm::dvec3 eye = position_;
    viewMatrix_ = glm::lookAt(eye, eye + forward, up);
}

void Camera::RecalculateProjectionMatrix() {
    projectionMatrix_ = glm::perspective(glm::radians(fovY_), aspectRatio_, nearPlane_, farPlane_);
}

glm::dvec3 Camera::screenToWorldRay(float screenX, float screenY, float viewportWidth, float viewportHeight) {
    // Convert to NDC [-1,1]
    const double ndcX = (2.0 * static_cast<double>(screenX)) / static_cast<double>(viewportWidth) - 1.0;
    const double ndcY = 1.0 - (2.0 * static_cast<double>(screenY)) / static_cast<double>(viewportHeight);

    // Camera basis from yaw/pitch
    const double pitchRad = DegToRad(pitch_);
    const double yawRad   = DegToRad(yaw_);
    const glm::dvec3 forward = glm::normalize(glm::dvec3(
        std::cos(pitchRad) * std::sin(yawRad),
        std::sin(pitchRad),
        -std::cos(pitchRad) * std::cos(yawRad)));
    const glm::dvec3 worldUp(0.0, 1.0, 0.0);
    const glm::dvec3 right = glm::normalize(glm::cross(forward, worldUp));
    const glm::dvec3 up = glm::normalize(glm::cross(right, forward));

    // Convert NDC to camera-space ray with FOV
    const double tanHalfFovY = std::tan(glm::radians(fovY_) * 0.5);
    const double tanHalfFovX = tanHalfFovY * static_cast<double>(aspectRatio_);

    glm::dvec3 dirCamera(
        ndcX * tanHalfFovX,
        ndcY * tanHalfFovY,
        -1.0
    );
    dirCamera = glm::normalize(dirCamera);

    // Transform from camera space to world space using basis
    glm::dvec3 worldDir = glm::normalize(dirCamera.x * right + dirCamera.y * up + dirCamera.z * forward);
    return worldDir;
}

} // namespace Scene
} // namespace Arche