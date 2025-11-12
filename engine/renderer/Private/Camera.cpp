#pragma once
#include "World.h"
#include "Camera.h"
#include <ISubsystem.h>
#include <JobPoolService.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <cmath>
#include <memory>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Arche {
namespace Scene {

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

glm::dmat4 Camera::GetViewMatrix() {
    RecalculateViewMatrix();
    return viewMatrix_;
}

glm::dmat4 Camera::GetProjectionMatrix() {
    RecalculateProjectionMatrix();
    return projectionMatrix_;
}

void Camera::RecalculateViewMatrix() {
    // Compute forward vector from pitch/yaw (degrees) - keep same convention as before:
    // yaw = 0 looks down -Z, positive yaw rotates to the right.
    const double pitchRad = DegToRad(pitch_);
    const double yawRad = DegToRad(yaw_);

    const double fx = std::cos(pitchRad) * std::sin(yawRad);
    const double fy = std::sin(pitchRad);
    const double fz = -std::cos(pitchRad) * std::cos(yawRad);
    glm::dvec3 forward = glm::dvec3(fx, fy, fz);
    forward = glm::normalize(forward);

    // World up (Y)
    glm::dvec3 worldUp = glm::dvec3(0.0, 1.0, 0.0);

    // Right and up
    glm::dvec3 right = glm::normalize(glm::cross(forward, worldUp));
    glm::dvec3 up = glm::normalize(glm::cross(right, forward));

    // Build view matrix using GLM (right-handed lookAt)
    glm::dvec3 eye = position_;
    glm::dmat4 view = glm::lookAt(eye, eye + forward, up);

    viewMatrix_ = view;
}

void Camera::RecalculateProjectionMatrix() {
    // GLM perspective uses radians
    glm::dmat4 proj = glm::perspective(glm::radians(fovY_), aspectRatio_, nearPlane_, farPlane_);
    projectionMatrix_ = proj;
}

glm::dvec3 Camera::screenToWorldRay(float screenX, float screenY, float viewportWidth, float viewportHeight) {
    // Convert to normalized device coordinates (NDC)
    double ndcX = (2.0 * static_cast<double>(screenX)) / static_cast<double>(viewportWidth) - 1.0;
    double ndcY = 1.0 - (2.0 * static_cast<double>(screenY)) / static_cast<double>(viewportHeight);

    // Clip space points
    glm::dvec4 clipNear(ndcX, ndcY, -1.0, 1.0);
    glm::dvec4 clipFar (ndcX, ndcY,  1.0, 1.0);

    // Use stored GLM matrices directly
    glm::dmat4 viewMat = viewMatrix_;
    glm::dmat4 projMat = projectionMatrix_;

    glm::dmat4 invVP = glm::inverse(projMat * viewMat);

    glm::dvec4 worldNearH = invVP * clipNear;
    glm::dvec4 worldFarH  = invVP * clipFar;

    // Perspective divide
    glm::dvec3 worldNear = glm::dvec3(worldNearH) / worldNearH.w;
    glm::dvec3 worldFar  = glm::dvec3(worldFarH)  / worldFarH.w;

    glm::dvec3 dir = glm::normalize(worldFar - worldNear);
    return dir;
}

} // namespace Scene
} // namespace Arche