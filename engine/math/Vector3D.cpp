#include "Vector3D.h"

#include <sstream>
#include <stdexcept>

// Arithmetic operators

namespace Arche {
namespace Math {

Vector3D Vector3D::operator+(const Vector3D& other) const {
  return Vector3D(x_comp + other.x_comp, y_comp + other.y_comp,
                  z_comp + other.z_comp);
}

Vector3D Vector3D::operator-(const Vector3D& other) const {
  return Vector3D(x_comp, y_comp, z_comp) -= other;
}

Vector3D Vector3D::operator*(double scalar) const {
  return Vector3D(x_comp * scalar, y_comp * scalar, z_comp * scalar);
}

Vector3D Vector3D::operator/(double scalar) const {
  if (scalar == 0.0) throw std::runtime_error("Division by zero");
  return Vector3D(x_comp / scalar, y_comp / scalar, z_comp / scalar);
}

Vector3D& Vector3D::operator+=(const Vector3D& other) {
  x_comp += other.x();
  y_comp += other.y();
  z_comp += other.z();
  return *this;
}

Vector3D& Vector3D::operator-=(const Vector3D& other) {
  x_comp -= other.x();
  y_comp -= other.y();
  z_comp -= other.z();

  return *this;
}

Vector3D& Vector3D::operator*=(double scalar) {
  x_comp *= scalar;
  y_comp *= scalar;
  z_comp *= scalar;
  return *this;
}

Vector3D& Vector3D::operator/=(double scalar) {
  if (scalar == 0.0) throw std::runtime_error("Division by zero");
  x_comp /= scalar;
  y_comp /= scalar;
  z_comp /= scalar;
  return *this;
}

double Vector3D::length() const {
  return std::sqrt(x_comp * x_comp + y_comp * y_comp + z_comp * z_comp);
}

Vector3D Vector3D::normalized() const {
  double len =
      std::sqrt(x_comp * x_comp + y_comp * y_comp + z_comp * z_comp);
  if (len == 0.0) throw std::runtime_error("Cannot normalize zero vector");
  return *this / len;
}

double Vector3D::dot(const Vector3D& other) const {
  return x_comp * other.x_comp + y_comp * other.y_comp +
         z_comp * other.z_comp;
}

Vector3D Vector3D::cross(const Vector3D& other) const {
  return Vector3D(y_comp * other.z_comp - z_comp * other.y_comp,
                  z_comp * other.x_comp - x_comp * other.z_comp,
                  x_comp * other.y_comp - y_comp * other.x_comp);
}

std::string Vector3D::to_string() const {
  std::ostringstream oss;
  oss << "(" << x_comp << "," << y_comp << "," << z_comp << ")";
  return oss.str();
}

}  // namespace Math
}  // namespace Arche