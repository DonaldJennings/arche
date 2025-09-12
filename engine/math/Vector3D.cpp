#include "Vector3D.h"
#include <cmath>
#include <sstream>
#include <stdexcept>

// Arithmetic operators

namespace Arche {
namespace Math {

Vector3D Vector3D::operator+(const Vector3D& other) const {
  return Vector3D(x_property + other.x_property, y_property + other.y_property,
                  z_property + other.z_property);
}

Vector3D Vector3D::operator-(const Vector3D& other) const {
  return Vector3D(x_property, y_property, z_property) -= other;
}

Vector3D Vector3D::operator*(double scalar) const {
  return Vector3D(x_property * scalar, y_property * scalar, z_property * scalar);
}

Vector3D Vector3D::operator/(double scalar) const {
  if (scalar == 0.0) throw std::runtime_error("Division by zero");
  return Vector3D(x_property / scalar, y_property / scalar, z_property / scalar);
}

Vector3D& Vector3D::operator+=(const Vector3D& other) {
  x_property += other.x();
  y_property += other.y();
  z_property += other.z();
  return *this;
}

Vector3D& Vector3D::operator-=(const Vector3D& other) {
  x_property -= other.x();
  y_property -= other.y();
  z_property -= other.z();

  return *this;
}

Vector3D& Vector3D::operator*=(double scalar) {
  x_property *= scalar;
  y_property *= scalar;
  z_property *= scalar;
  return *this;
}

Vector3D& Vector3D::operator/=(double scalar) {
  if (scalar == 0.0) throw std::runtime_error("Division by zero");
  x_property /= scalar;
  y_property /= scalar;
  z_property /= scalar;
  return *this;
}

Vector3D Vector3D::normalized() const {
  double len = std::sqrt(x_property * x_property + y_property * y_property + z_property * z_property);
  if (len == 0.0) throw std::runtime_error("Cannot normalize zero vector");
  return *this / len;
}

double Vector3D::dot(const Vector3D& other) const {
  return x_property * other.x_property + y_property * other.y_property + z_property * other.z_property;
}

Vector3D Vector3D::cross(const Vector3D& other) const {
  return Vector3D(
    y_property * other.z_property - z_property * other.y_property,
    z_property * other.x_property - x_property * other.z_property,
    x_property * other.y_property - y_property * other.x_property
  );
}

std::string Vector3D::to_string() const {
  std::ostringstream oss;
  oss << "(" << x_property << "," << y_property << "," << z_property << ")";
  return oss.str();
}

}  // namespace Math
}  // namespace Arche