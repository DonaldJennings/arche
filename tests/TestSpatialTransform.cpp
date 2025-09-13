#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "math/Vector3D.h"
#include "math/Quaternion.h"
#include "math/SpatialTransform.h"

using Arche::Math::SpatialTransform;
using Arche::Math::Vector3D;
using Arche::Math::Quaternion;

TEST_CASE("SpatialTransform default construction", "[vectortransformation]") {
    SpatialTransform t;
    REQUIRE(t.getPosition().x() == Catch::Approx(0.0));
    REQUIRE(t.getPosition().y() == Catch::Approx(0.0));
    REQUIRE(t.getPosition().z() == Catch::Approx(0.0));

    REQUIRE(t.getRotation().w() == Catch::Approx(0.0));
    REQUIRE(t.getRotation().x() == Catch::Approx(0.0));
    REQUIRE(t.getRotation().y() == Catch::Approx(0.0));
    REQUIRE(t.getRotation().z() == Catch::Approx(1.0));
}

TEST_CASE("SpatialTransform scale default", "[vectortransformation]") {
    SpatialTransform t;
    REQUIRE(t.getScale().x() == Catch::Approx(1.0));
    REQUIRE(t.getScale().y() == Catch::Approx(1.0));
    REQUIRE(t.getScale().z() == Catch::Approx(1.0));
}

TEST_CASE("SpatialTransform scale construction", "[vectortransformation]") {
    Vector3D pos(1.0, 2.0, 3.0);
    Quaternion rot(0.5, 0.5, 0.5, 0.5);
    Vector3D scale(2.0, 3.0, 4.0);
    SpatialTransform t(pos, rot, scale);
    REQUIRE(t.getPosition().x() == Catch::Approx(1.0));
    REQUIRE(t.getPosition().y() == Catch::Approx(2.0));
    REQUIRE(t.getPosition().z() == Catch::Approx(3.0));
    REQUIRE(t.getRotation().w() == Catch::Approx(0.5));
    REQUIRE(t.getRotation().x() == Catch::Approx(0.5));
    REQUIRE(t.getRotation().y() == Catch::Approx(0.5));
    REQUIRE(t.getRotation().z() == Catch::Approx(0.5));
    REQUIRE(t.getScale().x() == Catch::Approx(2.0));
    REQUIRE(t.getScale().y() == Catch::Approx(3.0));
    REQUIRE(t.getScale().z() == Catch::Approx(4.0));
}

TEST_CASE("SpatialTransform value construction", "[vectortransformation]") {
    Vector3D pos(1.0, 2.0, 3.0);
    Quaternion rot(0.5, 0.5, 0.5, 0.5);
    SpatialTransform t(pos, rot);

    REQUIRE(t.getPosition().x() == Catch::Approx(1.0));
    REQUIRE(t.getPosition().y() == Catch::Approx(2.0));
    REQUIRE(t.getPosition().z() == Catch::Approx(3.0));

    REQUIRE(t.getRotation().w() == Catch::Approx(0.5));
    REQUIRE(t.getRotation().x() == Catch::Approx(0.5));
    REQUIRE(t.getRotation().y() == Catch::Approx(0.5));
    REQUIRE(t.getRotation().z() == Catch::Approx(0.5));
}

TEST_CASE("SpatialTransform set/get position", "[vectortransformation]") {
    SpatialTransform t;
    Vector3D pos(4.0, 5.0, 6.0);
    t.setPosition(pos);

    REQUIRE(t.getPosition().x() == Catch::Approx(4.0));
    REQUIRE(t.getPosition().y() == Catch::Approx(5.0));
    REQUIRE(t.getPosition().z() == Catch::Approx(6.0));
}

TEST_CASE("SpatialTransform set/get rotation", "[vectortransformation]") {
    SpatialTransform t;
    Quaternion rot(0.1, 0.2, 0.3, 0.4);
    t.setRotation(rot);

    REQUIRE(t.getRotation().w() == Catch::Approx(0.1));
    REQUIRE(t.getRotation().x() == Catch::Approx(0.2));
    REQUIRE(t.getRotation().y() == Catch::Approx(0.3));
    REQUIRE(t.getRotation().z() == Catch::Approx(0.4));
}

TEST_CASE("SpatialTransform set/get scale", "[vectortransformation]") {
    SpatialTransform t;
    Vector3D scale(2.0, 3.0, 4.0);
    t.setScale(scale);

    REQUIRE(t.getScale().x() == Catch::Approx(2.0));
    REQUIRE(t.getScale().y() == Catch::Approx(3.0));
    REQUIRE(t.getScale().z() == Catch::Approx(4.0));
}