#pragma once
#include "World.h"
#include "Camera.h"
#include <ISubsystem.h>
#include <JobPoolService.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <cmath>
#include <memory>

namespace
{
    constexpr double M_PI = 3.14159265358979323846;
}

namespace Arche {
namespace Scene {

// Helper
static double DegToRad(double d) { return d * (M_PI / 180.0); }

void Camera::SetPosition(const Math::Vector3D &position) {
    position_ = position;
    // Recalculate lazily on next GetViewMatrix call
    RecalculateViewMatrix();
}

 Math::Vector3D Camera::GetPosition() {
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

std::array<double, 16> Camera::GetViewMatrix() {
    // Ensure view matrix is up to date
    RecalculateViewMatrix();
    return viewMatrix_;
}

std::array<double, 16> Camera::GetProjectionMatrix() {
    RecalculateProjectionMatrix();
    return projectionMatrix_;
}

void Camera::RecalculateViewMatrix() {
    // Compute forward vector from pitch/yaw (degrees)
    const double pitchRad = DegToRad(pitch_);
    const double yawRad = DegToRad(yaw_);

    // Use convention: yaw = 0 looks down -Z, positive yaw rotates to the right.
    // forward:
    double fx = std::cos(pitchRad) * std::sin(yawRad);
    double fy = std::sin(pitchRad);
    double fz = -std::cos(pitchRad) * std::cos(yawRad);
    Math::Vector3D forward(fx, fy, fz);
    forward = forward.normalized();

    // World up
    Math::Vector3D worldUp = Math::Vector3D::UnitY();

    // Right = normalize(cross(forward, worldUp))
    Math::Vector3D right = forward.cross(worldUp).normalized();

    // Recomputed up = cross(right, forward)
    Math::Vector3D up = right.cross(forward).normalized();

    // Build lookAt style view matrix (column-major)
    // f = forward
    // s = right
    // u = up
    const Math::Vector3D &f = forward;
    const Math::Vector3D &s = right;
    const Math::Vector3D &u = up;

    // translation components
    double tx = - (s.x() * position_.x() + s.y() * position_.y() + s.z() * position_.z());
    double ty = - (u.x() * position_.x() + u.y() * position_.y() + u.z() * position_.z());
    double tz =   (f.x() * position_.x() + f.y() * position_.y() + f.z() * position_.z());

    // Column-major layout: m[col*4 + row]
    std::array<double, 16> m{};
    m[0]  = s.x(); m[1]  = s.y(); m[2]  = s.z(); m[3]  = tx;
    m[4]  = u.x(); m[5]  = u.y(); m[6]  = u.z(); m[7]  = ty;
    m[8]  = -f.x();m[9]  = -f.y();m[10] = -f.z();m[11] = tz;
    m[12] = 0.0;   m[13] = 0.0;   m[14] = 0.0;   m[15] = 1.0;

    const_cast<Camera*>(this)->viewMatrix_ = m;
}

void Camera::RecalculateProjectionMatrix() {
    // Perspective projection (column-major)
    const double fovRad = DegToRad(fovY_);
    const double f = 1.0 / std::tan(fovRad * 0.5);
    const double nf = 1.0 / (nearPlane_ - farPlane_);

    std::array<double, 16> p{};
    p[0]  = f / aspectRatio_;
    p[1]  = 0.0;
    p[2]  = 0.0;
    p[3]  = 0.0;

    p[4]  = 0.0;
    p[5]  = f;
    p[6]  = 0.0;
    p[7]  = 0.0;

    p[8]  = 0.0;
    p[9]  = 0.0;
    p[10] = (farPlane_ + nearPlane_) * nf;
    p[11] = (2.0 * farPlane_ * nearPlane_) * nf;

    p[12] = 0.0;
    p[13] = 0.0;
    p[14] = -1.0;
    p[15] = 0.0;

    // Note: layout chosen for a right-handed projection compatible with common GL-style usage.
    const_cast<Camera*>(this)->projectionMatrix_ = p;
}

} // namespace Scene
} // namespace Arche