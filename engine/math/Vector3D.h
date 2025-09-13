#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <string>

namespace Arche {
    namespace Math {

        /**
         * @brief A class representing a 3D vector with x, y, and z components.
         */
        class Vector3D {
          public:
            
            /**
             * @brief Default constructor that initializes the vector to (0.0,
             * 0.0, 0.0).
             */
            Vector3D() : x_comp(0.0), y_comp(0.0), z_comp(0.0) {}
            
            /**
             * @brief Constructs a Vector3D object with the specified x, y, and z components.
             * @param x The x component of the vector.
             * @param y The y component of the vector.
             * @param z The z component of the vector.
             */
            Vector3D(double x, double y, double z)
                : x_comp(x), y_comp(y), z_comp(z) {}

            /**
             * @brief Default copy constructor for the Vector3D class.
             * @param other The Vector3D instance to copy from.
             */
            Vector3D(const Vector3D &other) = default;

            /**
             * @brief Assigns the values from another Vector3D object to this object.
             * @param other The Vector3D object to copy values from.
             * @return A reference to this Vector3D object after assignment.
             */
            Vector3D &operator=(const Vector3D &other) = default;

            /**
             * @brief Adds this vector to another vector and returns the result.
             * @param other The vector to add to this vector.
             * @return A new Vector3D representing the sum of this vector and the other vector.
             */
            Vector3D operator+(const Vector3D &other) const;

            /**
             * @brief Returns the result of subtracting another Vector3D from this vector.
             * @param other The Vector3D to subtract from this vector.
             * @return A new Vector3D representing the difference between this vector and the specified vector.
             */
            Vector3D operator-(const Vector3D &other) const;

            /**
             * @brief Multiplies the vector by a scalar value.
             * @param scalar The scalar value to multiply the vector by.
             * @return A new Vector3D that is the result of scaling the original vector by the given scalar.
             */
            Vector3D operator*(double scalar) const;

            /**
             * @brief Divides the vector by a scalar value and returns the resulting vector.
             * @param scalar The scalar value by which to divide the vector.
             * @return A new Vector3D representing the result of dividing each component of the vector by the scalar.
             */
            Vector3D operator/(double scalar) const;

            /**
             * @brief Adds the components of another Vector3D to this vector and assigns the result to this vector.
             * @param other The Vector3D whose components are to be added.
             * @return A reference to this Vector3D after addition.
             */
            Vector3D &operator+=(const Vector3D &other);
            
            /**
             * @brief Subtracts the components of another Vector3D from this vector and assigns the result to this vector.
             * @param other The Vector3D to subtract from this vector.
             * @return A reference to this vector after subtraction.
             */
            Vector3D &operator-=(const Vector3D &other);

            /**
             * @brief Multiplies the vector by a scalar value and assigns the result to this vector.
             * @param scalar The scalar value to multiply the vector by.
             * @return A reference to this vector after multiplication.
             */
            Vector3D &operator*=(double scalar);

            /**
             * @brief Divides the vector by a scalar value and assigns the result to this vector.
             * @param scalar The scalar value by which to divide the vector.
             * @return A reference to this vector after division.
             */
            Vector3D &operator/=(double scalar);

            /**
             * @brief Returns the length or magnitude.
             * @return The length or magnitude as a double.
             */
            double length() const;
            
            /**
             * @brief Returns a normalized (unit length) version of the vector.
             * @return A new Vector3D object representing the normalized vector.
             */
            Vector3D normalized() const;

            /**
             * @brief Calculates the dot product of this vector and another Vector3D.
             * @param other The other Vector3D to compute the dot product with.
             * @return The dot product of the two vectors as a double.
             */
            double dot(const Vector3D &other) const;

            /**
             * @brief Computes the cross product of this vector and another vector.
             * @param other The other vector to compute the cross product with.
             * @return A new Vector3D representing the cross product of this vector and the other vector.
             */
            Vector3D cross(const Vector3D &other) const;

            // Accessors
            inline double x() const { return x_comp; }
            inline double y() const { return y_comp; }
            inline double z() const { return z_comp; }
            inline void set_x(double const x) { x_comp = x; }
            inline void set_y(double const y) { y_comp = y; }
            inline void set_z(double const z) { z_comp = z; }

            /**
             * @brief Converts the object to its string representation.
             * @return A string representing the object.
             */
            std::string to_string() const;

            // Static vectors
            static Vector3D Zero() { return Vector3D(0.0, 0.0, 0.0); }
            static Vector3D One() { return Vector3D(1.0, 1.0, 1.0); }
            static Vector3D UnitX() { return Vector3D(1.0, 0.0, 0.0); }
            static Vector3D UnitY() { return Vector3D(0.0, 1.0, 0.0); }
            static Vector3D UnitZ() { return Vector3D(0.0, 0.0, 1.0); }

          private:
            double x_comp, y_comp, z_comp;
        };

    } // namespace Math
} // namespace Arche
#endif