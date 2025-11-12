#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <PhysicsSystem.h>
#include <RigidBody.h>
#include <LoggingService.h>
#include <glm/glm.hpp>
#include <memory>

using namespace Arche::Physics;
using namespace Catch;

SCENARIO("PhysicsSystem integrates gravity and respects static bodies", "[physics]") {
    auto logger = std::make_shared<Arche::Core::LoggingService>();
    PhysicsSystem physics(logger);

    GIVEN("a dynamic body with gravity enabled") {
        glm::vec3 pos{0.0f, 10.0f, 0.0f};
        auto rb = std::make_shared<RigidBody>(1.0f);
        rb->setUseGravity(true);
        rb->setStatic(false);
        PhysicsBody body{rb, nullptr, &pos};
        physics.addBody(body);

        WHEN("updated for some time") {
            physics.update(0.1);
            THEN("y position decreases") {
                CHECK(pos.y < 10.0f);
            }
        }
    }

    GIVEN("a static body") {
        glm::vec3 pos{0.0f, 10.0f, 0.0f};
        auto rb = std::make_shared<RigidBody>(1.0f);
        rb->setStatic(true);
        PhysicsBody body{rb, nullptr, &pos};
        physics.addBody(body);

        WHEN("updated") {
            auto before = pos;
            physics.update(0.1);
            THEN("position remains unchanged") {
                CHECK(pos.x == Approx(before.x));
                CHECK(pos.y == Approx(before.y));
                CHECK(pos.z == Approx(before.z));
            }
        }
    }
}
