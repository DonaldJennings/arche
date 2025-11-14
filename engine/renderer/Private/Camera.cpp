#include "Camera.h"

namespace Arche {
    namespace Render {

        void Camera::SetPosition(const glm::dvec3 &position) {
            position_ = position;
            RecalculateViewMatrix();
        }

        glm::dvec3 Camera::GetPosition() const { return position_; }

        void Camera::SetPitch(double pitch) {
            pitch_ = pitch;
            RecalculateViewMatrix();
        }

        double Camera::GetPitch() const { return pitch_; }

        void Camera::SetYaw(double yaw) {
            yaw_ = yaw;
            RecalculateViewMatrix();
        }

        double Camera::GetYaw() const { return yaw_; }

        void Camera::setPitchYaw(double pitch, double yaw) {
            pitch_ = pitch;
            yaw_ = yaw;
            RecalculateViewMatrix();
        }

        glm::dquat Camera::GetOrientation() const {
            // Construct quaternion from Euler angles (yaw and pitch)
            // Note: GLM uses radians for trigonometric functions
            return glm::dquat(glm::dvec3(glm::radians(pitch_), glm::radians(yaw_), 0.0));
        }

        void Camera::setPerspective(double fovY, double aspectRatio, double nearPlane, double farPlane) {
            fovY_ = fovY;
            aspectRatio_ = aspectRatio;
            nearPlane_ = nearPlane;
            farPlane_ = farPlane;
            RecalculateProjectionMatrix();
        }

        glm::dvec3 Camera::screenToWorldRay(float screenX, float screenY, float viewportWidth, float viewportHeight) {
            // Implementation for screenToWorldRay...
            return glm::dvec3(0.0);
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
            glm::dquat orientation = GetOrientation();
            viewMatrix_ = glm::translate(glm::dmat4(1.0), position_) * glm::mat4_cast(orientation);
            viewMatrix_ = glm::inverse(viewMatrix_);
        }

        void Camera::RecalculateProjectionMatrix() {
            projectionMatrix_ = glm::perspective(glm::radians(fovY_), aspectRatio_, nearPlane_, farPlane_);
        }

    } // namespace Render
} // namespace Arche