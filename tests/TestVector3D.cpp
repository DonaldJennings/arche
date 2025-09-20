#include <math/Vector3D.h>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

// Construction and accessors
TEST_CASE("Arche::Math::Vector3D DefaultConstructor", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v;
  CHECK(v.x() == Catch::Approx(0.0));
  CHECK(v.y() == Catch::Approx(0.0));
  CHECK(v.z() == Catch::Approx(0.0));
}

TEST_CASE("Arche::Math::Vector3D ParameterizedConstructor", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v(1.1, 2.2, 3.3);
  CHECK(v.x() == Catch::Approx(1.1));
  CHECK(v.y() == Catch::Approx(2.2));
  CHECK(v.z() == Catch::Approx(3.3));
}

TEST_CASE("Arche::Math::Vector3D CopyConstructor", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v1(4.0, 5.0, 6.0);
  Arche::Math::Vector3D v2(v1);
  CHECK(v2.x() == Catch::Approx(4.0));
  CHECK(v2.y() == Catch::Approx(5.0));
  CHECK(v2.z() == Catch::Approx(6.0));
}

TEST_CASE("Arche::Math::Vector3D AssignmentOperator", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v1(7.0, 8.0, 9.0);
  Arche::Math::Vector3D v2;
  v2 = v1;
  CHECK(v2.x() == Catch::Approx(7.0));
  CHECK(v2.y() == Catch::Approx(8.0));
  CHECK(v2.z() == Catch::Approx(9.0));
}

// Arithmetic operators
TEST_CASE("Arche::Math::Vector3D Addition", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v1(1, 2, 3);
  Arche::Math::Vector3D v2(4, 5, 6);
  Arche::Math::Vector3D v3 = v1 + v2;
  CHECK(v3.x() == Catch::Approx(5));
  CHECK(v3.y() == Catch::Approx(7));
  CHECK(v3.z() == Catch::Approx(9));
}

TEST_CASE("Arche::Math::Vector3D Subtraction", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v1(5, 7, 9);
  Arche::Math::Vector3D v2(1, 2, 3);
  Arche::Math::Vector3D v3 = v1 - v2;
  CHECK(v3.x() == Catch::Approx(4));
  CHECK(v3.y() == Catch::Approx(5));
  CHECK(v3.z() == Catch::Approx(6));
}

TEST_CASE("Arche::Math::Vector3D ScalarMultiplication", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v(1, -2, 3);
  Arche::Math::Vector3D result = v * 2.5;
  CHECK(result.x() == Catch::Approx(2.5));
  CHECK(result.y() == Catch::Approx(-5.0));
  CHECK(result.z() == Catch::Approx(7.5));
}

TEST_CASE("Arche::Math::Vector3D ScalarDivision", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v(2, -4, 6);
  Arche::Math::Vector3D result = v / 2.0;
  CHECK(result.x() == Catch::Approx(1.0));
  CHECK(result.y() == Catch::Approx(-2.0));
  CHECK(result.z() == Catch::Approx(3.0));
}

TEST_CASE("Arche::Math::Vector3D CompoundAddition", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v1(1, 2, 3);
  Arche::Math::Vector3D v2(4, 5, 6);
  v1 += v2;
  CHECK(v1.x() == Catch::Approx(5));
  CHECK(v1.y() == Catch::Approx(7));
  CHECK(v1.z() == Catch::Approx(9));
}

TEST_CASE("Arche::Math::Vector3D CompoundSubtraction", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v1(5, 7, 9);
  Arche::Math::Vector3D v2(1, 2, 3);
  v1 -= v2;
  CHECK(v1.x() == Catch::Approx(4));
  CHECK(v1.y() == Catch::Approx(5));
  CHECK(v1.z() == Catch::Approx(6));
}

TEST_CASE("Arche::Math::Vector3D CompoundMultiplication", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v(1, -2, 3);
  v *= 3.0;
  CHECK(v.x() == Catch::Approx(3.0));
  CHECK(v.y() == Catch::Approx(-6.0));
  CHECK(v.z() == Catch::Approx(9.0));
}

TEST_CASE("Arche::Math::Vector3D CompoundDivision", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v(2, -4, 6);
  v /= 2.0;
  CHECK(v.x() == Catch::Approx(1.0));
  CHECK(v.y() == Catch::Approx(-2.0));
  CHECK(v.z() == Catch::Approx(3.0));
}

// Vector math
TEST_CASE("Arche::Math::Vector3D Length", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v(3, 4, 12);
  CHECK(v.length() == Catch::Approx(13.0));
}

TEST_CASE("Arche::Math::Vector3D Normalized", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v(0, 3, 4);
  Arche::Math::Vector3D n = v.normalized();
  CHECK(n.x() == Catch::Approx(0.0).margin(1e-9));
  CHECK(n.y() == Catch::Approx(0.6).margin(1e-9));
  CHECK(n.z() == Catch::Approx(0.8).margin(1e-9));
  CHECK(n.length() == Catch::Approx(1.0).margin(1e-9));
}

TEST_CASE("Arche::Math::Vector3D DotProduct", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v1(1, 2, 3);
  Arche::Math::Vector3D v2(4, -5, 6);
  CHECK(v1.dot(v2) == Catch::Approx(12.0));
}

TEST_CASE("Arche::Math::Vector3D CrossProduct", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v1(1, 2, 3);
  Arche::Math::Vector3D v2(4, 5, 6);
  Arche::Math::Vector3D cross = v1.cross(v2);
  CHECK(cross.x() == Catch::Approx(-3.0));
  CHECK(cross.y() == Catch::Approx(6.0));
  CHECK(cross.z() == Catch::Approx(-3.0));
}

// Mutators
TEST_CASE("Arche::Math::Vector3D Setters", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v;
  v.set_x(7.7);
  v.set_y(8.8);
  v.set_z(9.9);
  CHECK(v.x() == Catch::Approx(7.7));
  CHECK(v.y() == Catch::Approx(8.8));
  CHECK(v.z() == Catch::Approx(9.9));
}

// String representation
TEST_CASE("Arche::Math::Vector3D ToString", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D v(1.23, 4.56, 7.89);
  std::string s = v.to_string();
  CHECK(s.find("1.23") != std::string::npos);
  CHECK(s.find("4.56") != std::string::npos);
  CHECK(s.find("7.89") != std::string::npos);
}

// Static vectors
TEST_CASE("Arche::Math::Vector3D StaticVectors", "[Arche::Math::Vector3D]") {
  Arche::Math::Vector3D zero = Arche::Math::Vector3D::Zero();
  CHECK(zero.x() == Catch::Approx(0.0));
  CHECK(zero.y() == Catch::Approx(0.0));
  CHECK(zero.z() == Catch::Approx(0.0));

  Arche::Math::Vector3D one = Arche::Math::Vector3D::One();
  CHECK(one.x() == Catch::Approx(1.0));
  CHECK(one.y() == Catch::Approx(1.0));
  CHECK(one.z() == Catch::Approx(1.0));

  Arche::Math::Vector3D unitX = Arche::Math::Vector3D::UnitX();
  CHECK(unitX.x() == Catch::Approx(1.0));
  CHECK(unitX.y() == Catch::Approx(0.0));
  CHECK(unitX.z() == Catch::Approx(0.0));

  Arche::Math::Vector3D unitY = Arche::Math::Vector3D::UnitY();
  CHECK(unitY.x() == Catch::Approx(0.0));
  CHECK(unitY.y() == Catch::Approx(1.0));
  CHECK(unitY.z() == Catch::Approx(0.0));

  Arche::Math::Vector3D unitZ = Arche::Math::Vector3D::UnitZ();
  CHECK(unitZ.x() == Catch::Approx(0.0));
  CHECK(unitZ.y() == Catch::Approx(0.0));
  CHECK(unitZ.z() == Catch::Approx(1.0));
}