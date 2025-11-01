#ifndef ARCHE_QUATERNION_H
#define ARCHE_QUATERNION_H

#include <cmath>
#include <stdexcept>
#include <string>
#include "Vector3D.h"

namespace Arche {
namespace Math {

class Quaternion {
 public:
  // Components: w + xi + yj + zk
  Quaternion() : w_(1), x_(0), y_(0), z_(0) {}
  Quaternion(double w, double x, double y, double z) : w_(w), x_(x), y_(y), z_(z) {}
  Quaternion(double w, const Vector3D& v) : w_(w), x_(v.x()), y_(v.y()), z_(v.z()) {}

  // Copy and assignment
  Quaternion(const Quaternion&) = default;
  Quaternion& operator=(const Quaternion&) = default;

  // Arithmetic
  inline Quaternion operator+(const Quaternion& rhs) const {
    return Quaternion(w_ + rhs.w_, x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_);
  }
  inline Quaternion operator-(const Quaternion& rhs) const {
    return Quaternion(w_ - rhs.w_, x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_);
  }
  inline Quaternion operator*(double scalar) const {
    return Quaternion(w_ * scalar, x_ * scalar, y_ * scalar, z_ * scalar);
  }
  inline Quaternion operator/(double scalar) const {
    if (scalar == 0.0) throw std::runtime_error("Division by zero");
    return Quaternion(w_ / scalar, x_ / scalar, y_ / scalar, z_ / scalar);
  }
  Quaternion operator*(const Quaternion& rhs) const;

  Quaternion& operator+=(const Quaternion& rhs);
  Quaternion& operator-=(const Quaternion& rhs);
  Quaternion& operator*=(double scalar);

  Quaternion& operator/=(double scalar);
  Quaternion& operator*=(const Quaternion& rhs);

  // Norm and normalization
  inline double norm() const { return std::sqrt(w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_); }

  Quaternion normalized() const;

  // Conjugate and inverse
  inline Quaternion conjugate() const { return Quaternion(w_, -x_, -y_, -z_); }

  Quaternion inverse() const;

  // Rotate a vector
  Vector3D rotate(const Vector3D& v) const;

  // Accessors
  inline double w() const { return w_; }
  inline double x() const { return x_; }
  inline double y() const { return y_; }
  inline double z() const { return z_; }

  // String representation
  std::string to_string() const;

  // Static identity
  inline static Quaternion Identity() { return Quaternion(1, 0, 0, 0); }

 private:
  double w_, x_, y_, z_;
};

}  // namespace Math
}  // namespace Arche

#endif