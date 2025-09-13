#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <string>


namespace Arche {
namespace Math {

class Vector3D {
 public:
  // Constructors
  Vector3D() : x_comp(0.0), y_comp(0.0), z_comp(0.0) {}
  Vector3D(double x, double y, double z) : x_comp(x), y_comp(y), z_comp(z) {}

  // Copy constructor
  Vector3D(const Vector3D& other) = default;

  // Assignment operator
  Vector3D& operator=(const Vector3D& other) = default;

  // Arithmetic operators
  Vector3D operator+(const Vector3D& other) const;
  Vector3D operator-(const Vector3D& other) const;
  Vector3D operator*(double scalar) const;

  Vector3D operator/(double scalar) const;

  Vector3D& operator+=(const Vector3D& other);
  Vector3D& operator-=(const Vector3D& other);
  Vector3D& operator*=(double scalar);
  Vector3D& operator/=(double scalar);

  // Vector math
  double length() const;
  Vector3D normalized() const;
  double dot(const Vector3D& other) const;
  Vector3D cross(const Vector3D& other) const;

  // Accessors
  inline double x() const { return x_comp; }
  inline double y() const { return y_comp; }
  inline double z() const { return z_comp; }
  inline void set_x(double const x) { x_comp = x; }
  inline void set_y(double const y) { y_comp = y; }
  inline void set_z(double const z) { z_comp = z; }

  // String representation
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

}  // namespace Math
}  // namespace Arche
#endif