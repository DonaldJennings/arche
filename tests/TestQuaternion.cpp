#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "math/Quaternion.h"
#include "math/Vector3D.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using Arche::Math::Quaternion;
using Arche::Math::Vector3D;

TEST_CASE("Quaternion default and value construction", "[quaternion]") {
    Quaternion q1;
    REQUIRE(q1.w() == Catch::Approx(1.0));
    REQUIRE(q1.x() == Catch::Approx(0.0));
    REQUIRE(q1.y() == Catch::Approx(0.0));
    REQUIRE(q1.z() == Catch::Approx(0.0));

    Quaternion q2(0.5, 1.0, -2.0, 3.0);
    REQUIRE(q2.w() == Catch::Approx(0.5));
    REQUIRE(q2.x() == Catch::Approx(1.0));
    REQUIRE(q2.y() == Catch::Approx(-2.0));
    REQUIRE(q2.z() == Catch::Approx(3.0));
}

TEST_CASE("Quaternion arithmetic", "[quaternion]") {
    Quaternion q1(1, 2, 3, 4);
    Quaternion q2(0.5, -1, 2, -0.5);

    auto q_add = q1 + q2;
    REQUIRE(q_add.w() == Catch::Approx(1.5));
    REQUIRE(q_add.x() == Catch::Approx(1.0));
    REQUIRE(q_add.y() == Catch::Approx(5.0));
    REQUIRE(q_add.z() == Catch::Approx(3.5));

    auto q_sub = q1 - q2;
    REQUIRE(q_sub.w() == Catch::Approx(0.5));
    REQUIRE(q_sub.x() == Catch::Approx(3.0));
    REQUIRE(q_sub.y() == Catch::Approx(1.0));
    REQUIRE(q_sub.z() == Catch::Approx(4.5));

    auto q_mul = q1 * 2.0;
    REQUIRE(q_mul.w() == Catch::Approx(2.0));
    REQUIRE(q_mul.x() == Catch::Approx(4.0));
    REQUIRE(q_mul.y() == Catch::Approx(6.0));
    REQUIRE(q_mul.z() == Catch::Approx(8.0));

    auto q_div = q1 / 2.0;
    REQUIRE(q_div.w() == Catch::Approx(0.5));
    REQUIRE(q_div.x() == Catch::Approx(1.0));
    REQUIRE(q_div.y() == Catch::Approx(1.5));
    REQUIRE(q_div.z() == Catch::Approx(2.0));
}

TEST_CASE("Quaternion Hamilton product", "[quaternion]") {
    Quaternion q1(1, 0, 1, 0);
    Quaternion q2(1, 0.5, 0.5, 0.75);

    auto q_prod = q1 * q2;
    REQUIRE(q_prod.w() == Catch::Approx(0.5));
    REQUIRE(q_prod.x() == Catch::Approx(1.25));
    REQUIRE(q_prod.y() == Catch::Approx(1.5));
    REQUIRE(q_prod.z() == Catch::Approx(0.25));
}

TEST_CASE("Quaternion normalization and norm", "[quaternion]") {
    Quaternion q(0, 3, 0, 4);
    REQUIRE(q.norm() == Catch::Approx(5.0));
    auto qn = q.normalized();
    REQUIRE(qn.norm() == Catch::Approx(1.0));
    REQUIRE(qn.w() == Catch::Approx(0.0));
    REQUIRE(qn.x() == Catch::Approx(0.6));
    REQUIRE(qn.y() == Catch::Approx(0.0));
    REQUIRE(qn.z() == Catch::Approx(0.8));
}

TEST_CASE("Quaternion conjugate and inverse", "[quaternion]") {
    Quaternion q(1, 2, 3, 4);
    auto qc = q.conjugate();
    REQUIRE(qc.w() == Catch::Approx(1.0));
    REQUIRE(qc.x() == Catch::Approx(-2.0));
    REQUIRE(qc.y() == Catch::Approx(-3.0));
    REQUIRE(qc.z() == Catch::Approx(-4.0));

    auto qi = q.inverse();
    auto prod = q * qi;
    REQUIRE(prod.w() == Catch::Approx(1.0));
    REQUIRE(prod.x() == Catch::Approx(0.0));
    REQUIRE(prod.y() == Catch::Approx(0.0));

    // Apply a margin due to floating point precision errors
    REQUIRE(prod.z() == Catch::Approx(0.0).margin(1e-12));
}

TEST_CASE("Quaternion rotates a vector", "[quaternion]") {
    // 90 degree rotation around Z axis
    double angle = M_PI / 2;
    double s = std::sin(angle / 2);
    double c = std::cos(angle / 2);
    Quaternion q(c, 0, 0, s); // (cos(theta/2), 0, 0, sin(theta/2))

    Vector3D v(1, 0, 0);
    Vector3D v_rot = q.rotate(v);

    REQUIRE(v_rot.x() == Catch::Approx(0.0).margin(1e-12));
    REQUIRE(v_rot.y() == Catch::Approx(1.0).margin(1e-12));
    REQUIRE(v_rot.z() == Catch::Approx(0.0).margin(1e-12));
}