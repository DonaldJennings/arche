#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "World.h"
#include "SpatialTransform.h"
#include "Vector3D.h"
#include "WorldConfig.h"

using Arche::Scene::World;
using Arche::Math::SpatialTransform;
using Arche::Math::Vector3D;
using Arche::Core::WorldConfig;

TEST_CASE("World integration kernel updates particle under gravity", "[worldContainer][integration]") {
    WorldConfig config;
    config.stepDuration = 1.0f; // 1 second timestep for easy math
    auto world = World::Create(config);

    // Place particle at origin, mass 1.0
    SpatialTransform initialTransform;
    initialTransform.setPosition(Vector3D(0.0, 0.0, 0.0));
    float mass = 1.0f;
    world->createParticle(initialTransform, mass);

    // Set gravity to (0, -10, 0)
    world->setGravity(Vector3D(0.0, -10.0, 0.0));

    // Step the world once
    world->step(world->stepDuration());

    // Get the updated state
    auto view = world->view();
    REQUIRE(view.bodies.size() == 1);

    // After 1s, v = v0 + a*dt = 0 + (-10)*1 = -10
    // p = p0 + v*dt = 0 + (-10)*1 = -10
    auto& body = view.bodies[0];
    REQUIRE(body.velocity.x() == Catch::Approx(0.0));
    REQUIRE(body.velocity.y() == Catch::Approx(-10.0));
    REQUIRE(body.velocity.z() == Catch::Approx(0.0));
    REQUIRE(body.transform.getPosition().x() == Catch::Approx(0.0));
    REQUIRE(body.transform.getPosition().y() == Catch::Approx(-10.0));
    REQUIRE(body.transform.getPosition().z() == Catch::Approx(0.0));
}