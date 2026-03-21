#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <PhysicsSystem.h>
#include <RigidBody.h>
#include <GlobalSettings.h>
#include <LoggingService.h>
#include <glm/glm.hpp>
#include <memory>

using namespace Arche::Physics;
using namespace Catch;

SCENARIO("PhysicsSystem integrates gravity and respects static bodies", "[physics]") {
    auto logger   = std::make_shared<Arche::Core::LoggingService>();
    auto settings = std::make_shared<Arche::Core::GlobalSettings>();
    PhysicsSystem physics(logger, settings);

    GIVEN("a dynamic body with gravity enabled") {
        auto rb = std::make_shared<RigidBody>(1.0f);
        rb->setUseGravity(true);
        rb->setStatic(false);

        PhysicsBody body;
        body.entityId = 1;
        body.rb       = rb;
        body.position = glm::vec3(0.0f, 10.0f, 0.0f);
        physics.addBody(body);

        WHEN("updated for some time") {
            physics.update(0.1);
            THEN("y position decreases") {
                CHECK(physics.getPositions()[0].y < 10.0f);
            }
        }
    }

    GIVEN("a static body") {
        auto rb = std::make_shared<RigidBody>(1.0f);
        rb->setStatic(true);

        PhysicsBody body;
        body.entityId = 2;
        body.rb       = rb;
        body.position = glm::vec3(0.0f, 10.0f, 0.0f);
        physics.addBody(body);

        WHEN("updated") {
            physics.update(0.1);
            THEN("position remains unchanged") {
                CHECK(physics.getPositions()[0].x == Approx(0.0f));
                CHECK(physics.getPositions()[0].y == Approx(10.0f));
                CHECK(physics.getPositions()[0].z == Approx(0.0f));
            }
        }
    }
}
