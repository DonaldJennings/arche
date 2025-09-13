#include "Quaternion.h"
namespace Arche {
    namespace Math {
        Quaternion Math::Quaternion::operator*(const Quaternion &rhs) const {
            // Hamilton product
            return Quaternion(
                w_ * rhs.w_ - x_ * rhs.x_ - y_ * rhs.y_ - z_ * rhs.z_,
                w_ * rhs.x_ + x_ * rhs.w_ + y_ * rhs.z_ - z_ * rhs.y_,
                w_ * rhs.y_ - x_ * rhs.z_ + y_ * rhs.w_ + z_ * rhs.x_,
                w_ * rhs.z_ + x_ * rhs.y_ - y_ * rhs.x_ + z_ * rhs.w_);
        }

        Quaternion &Quaternion::operator+=(const Quaternion &rhs) {
            w_ += rhs.w_;
            x_ += rhs.x_;
            y_ += rhs.y_;
            z_ += rhs.z_;
            return *this;
        }

        Quaternion &Quaternion::operator-=(const Quaternion &rhs) {
            w_ -= rhs.w_;
            x_ -= rhs.x_;
            y_ -= rhs.y_;
            z_ -= rhs.z_;
            return *this;
        }

        Quaternion &Quaternion::operator*=(double scalar) {
            w_ *= scalar;
            x_ *= scalar;
            y_ *= scalar;
            z_ *= scalar;
            return *this;
        }

        Quaternion &Quaternion::operator*=(const Quaternion &rhs) {
            *this = *this * rhs;
            return *this;
        }

        Quaternion &Quaternion::operator/=(double scalar) {
            if (scalar == 0.0)
                throw std::runtime_error("Division by zero");
            w_ /= scalar;
            x_ /= scalar;
            y_ /= scalar;
            z_ /= scalar;
            return *this;
        }

        Quaternion Quaternion::normalized() const {
            double n = norm();
            if (n == 0.0)
                throw std::runtime_error("Cannot normalize zero quaternion");
            return *this / n;
        }

        Quaternion Quaternion::inverse() const {
            double n2 = w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_;
            if (n2 == 0.0)
                throw std::runtime_error("Cannot invert zero quaternion");
            return conjugate() / n2;
        }

        Vector3D Quaternion::rotate(const Vector3D &v) const {
            Quaternion p(0, v);
            Quaternion result = (*this) * p * this->inverse();
            return Vector3D(result.x_, result.y_, result.z_);
        }

        std::string Quaternion::to_string() const {
            return "{" + std::to_string(w_) + ", " + std::to_string(x_) +
                   "i, " + std::to_string(y_) + "j, " + std::to_string(z_) +
                   "k}";
        }

    } // namespace Math

} // namespace Arche
