#pragma once
#include <array>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp> 

namespace Arche {
    namespace Render {

        /**
         * @brief Virtual camera for 3D scene rendering.
         * 
         * The Camera class manages the view and projection transformations needed
         * for rendering 3D scenes. It uses double precision internally for better
         * accuracy when working with large world coordinates.
         * 
         * The camera uses a first-person style orientation defined by pitch and yaw
         * angles, and supports perspective projection with configurable field of view,
         * aspect ratio, and near/far clipping planes.
         * 
         * Key features:
         * - Double precision position and matrices for accuracy
         * - Pitch/yaw orientation system
         * - Perspective projection configuration
         * - Screen-to-world ray casting for mouse picking
         */
        class Camera {
          public:
            /**
             * @brief Default constructor.
             * 
             * Creates a camera at origin looking along the negative Z axis with
             * default perspective settings (60° FOV, 16:9 aspect ratio).
             */
            Camera() = default;

            // Position using GLM double-precision vector
            /**
             * @brief Set the camera's world position.
             * @param position New position in world coordinates
             */
            void SetPosition(const glm::dvec3 &position);
            
            /**
             * @brief Get the camera's world position.
             * @return Current position vector
             */
            glm::dvec3 GetPosition() const;

            /**
             * @brief Set the camera's pitch (vertical rotation).
             * @param pitch Angle in degrees (clamped to ±89° to avoid gimbal lock)
             */
            void SetPitch(double pitch);
            
            /**
             * @brief Get the camera's pitch.
             * @return Pitch angle in degrees
             */
            double GetPitch() const;

            /**
             * @brief Set the camera's yaw (horizontal rotation).
             * @param yaw Angle in degrees
             */
            void SetYaw(double yaw);
            
            /**
             * @brief Get the camera's yaw.
             * @return Yaw angle in degrees
             */
            double GetYaw() const;

            /**
             * @brief Set both pitch and yaw simultaneously.
             * 
             * More efficient than calling SetPitch and SetYaw separately as it
             * only recalculates the view matrix once.
             * 
             * @param pitch Vertical rotation in degrees
             * @param yaw Horizontal rotation in degrees
             */
            void setPitchYaw(double pitch, double yaw);

            /**
             * @brief Get the camera's orientation as a quaternion.
             * 
             * Useful for interpolation and certain rotation operations.
             * 
             * @return Orientation quaternion
             */
            glm::dquat GetOrientation() const;

            /**
             * @brief Configure the perspective projection.
             * 
             * @param fovY Vertical field of view in degrees
             * @param aspectRatio Width/height ratio of the viewport
             * @param nearPlane Near clipping plane distance
             * @param farPlane Far clipping plane distance
             */
            void setPerspective(double fovY, double aspectRatio, double nearPlane, double farPlane);

            /**
             * @brief Convert screen coordinates to a world-space ray direction.
             * 
             * Useful for mouse picking and raycasting from screen coordinates.
             * The ray origin is the camera position.
             * 
             * @param screenX X coordinate in pixels (0 = left)
             * @param screenY Y coordinate in pixels (0 = top)
             * @param viewportWidth Width of the viewport in pixels
             * @param viewportHeight Height of the viewport in pixels
             * @return Normalized ray direction in world space
             */
            glm::dvec3 screenToWorldRay(float screenX, float screenY, float viewportWidth, float viewportHeight);

            /**
             * @brief Get the view transformation matrix.
             * 
             * Transforms from world space to camera space. Recalculated when
             * position or orientation changes.
             * 
             * @return 4x4 view matrix
             */
            glm::dmat4 GetViewMatrix() ;
            
            /**
             * @brief Get the projection transformation matrix.
             * 
             * Transforms from camera space to clip space. Recalculated when
             * perspective settings change.
             * 
             * @return 4x4 projection matrix
             */
            glm::dmat4 GetProjectionMatrix();

          private:
            /**
             * @brief Recalculate the view matrix from position and orientation.
             */
            void RecalculateViewMatrix();
            
            /**
             * @brief Recalculate the projection matrix from perspective settings.
             */
            void RecalculateProjectionMatrix();

            // Use GLM double-precision vector for position
            glm::dvec3 position_{0.0, 0.0, 0.0};    ///< Camera world position
            double pitch_{0.0};                      ///< Vertical rotation in degrees
            double yaw_{0.0};                        ///< Horizontal rotation in degrees

            double fovY_{60.0};                      ///< Field of view in degrees
            double aspectRatio_{16.0 / 9.0};        ///< Viewport width/height
            double nearPlane_{0.1};                  ///< Near clipping distance
            double farPlane_{1000.0};                ///< Far clipping distance

            glm::dmat4 viewMatrix_{};               ///< Cached view matrix
            glm::dmat4 projectionMatrix_{};         ///< Cached projection matrix
        };

    } // namespace Render
} // namespace Arche