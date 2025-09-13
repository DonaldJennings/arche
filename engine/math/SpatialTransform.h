#ifndef ARCHE_ENGINE_MATH_VECTOR_TRANSFORMATION_H
#define ARCHE_ENGINE_MATH_VECTOR_TRANSFORMATION_H

#include "Quaternion.h"
#include "Vector3D.h"

namespace Arche {
    namespace Math {
        /**
         * @brief A class representing a spatial transformation in 3D space,
         * including position, rotation, and scale.
         */
        class SpatialTransform {
          public:

            /**
             * @brief Default constructor that initializes the spatial transform to the identity transformation.
             */
            SpatialTransform()
                : SpatialTransform(Vector3D(0.0f, 0.0f, 0.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3D(1.0f, 1.0f, 1.0f)) {
            }

            /**
             * @brief Constructs a SpatialTransform with the specified position and rotation, using a default scale of (1.0f, 1.0f, 1.0f).
             * @param pos The position vector of the transform.
             * @param rot The rotation quaternion of the transform.
             */
            SpatialTransform(const Vector3D &pos, const Quaternion &rot)
                : SpatialTransform(pos, rot, Vector3D(1.0f, 1.0f, 1.0f)) {}

            /**
             * @brief Constructs a SpatialTransform with the specified position, rotation, and scale.
             * @param pos The position vector of the transform.
             * @param rot The rotation represented as a quaternion.
             * @param s The scale vector of the transform.
             */
            SpatialTransform(const Vector3D &pos, const Quaternion &rot, const Vector3D &s)
                : position(pos), rotation(rot), scale(s) {}

            /**
             * @brief Sets the position to the specified 3D vector.
             * @param pos The new position as a Vector3D object.
             */
            inline void setPosition(const Vector3D &pos) { position = pos; }
            
            /**
             * @brief Sets the rotation to the specified quaternion value.
             * @param rot The quaternion representing the new rotation.
             */
            inline void setRotation(const Quaternion &rot) { rotation = rot; }
            
            /**
             * @brief Sets the scale to the specified 3D vector.
             * @param s The new scale as a Vector3D object.
             */
            inline void setScale(const Vector3D &s) { scale = s; }

            /**
             * @brief Gets the current position of the transform.
             * @return The position as a Vector3D object.
             */
            inline const Vector3D &getPosition() const { return position; }
            
            /**
             * @brief Gets the current rotation of the transform.
             * @return The rotation as a Quaternion object.
             */
            inline const Quaternion &getRotation() const { return rotation; }

            /**
             * @brief Gets the current scale of the transform.
             * @return The scale as a Vector3D object.
             */
            inline const Vector3D &getScale() const { return scale; }


          private:

            /**
             * @brief Represents a position in 3D space.
             */
            Vector3D position;

            /**
             * @brief Represents a rotation in 3D space using a quaternion.
             */
            Quaternion rotation;
            
            /**
             * @brief Represents scaling factors along the x, y, and z axes.
             */
            Vector3D scale;
        };
    } // namespace Math
} // namespace Arche

#endif // ARCHE_ENGINE_MATH_VECTOR_TRANSFORMATION_H