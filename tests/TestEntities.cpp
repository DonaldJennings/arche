#include <catch2/catch_test_macros.hpp>
#include <SphereEntity.h>
#include <CubeEntity.h>
#include <PlaneEntity.h>
#include <glm/glm.hpp>

using namespace Arche::Scene;

SCENARIO("Entities expose transform and component APIs", "[entities]") {
    GIVEN("a SphereEntity") {
        SphereEntity s(1.0f, glm::vec3(0,0,0));
        THEN("transform setters/getters work") {
            s.setPosition(glm::vec3(1,2,3));
            s.setRotation(glm::vec3(4,5,6));
            s.setScale(glm::vec3(2));
            CHECK(s.getPosition() == glm::vec3(1,2,3));
            CHECK(s.getRotation() == glm::vec3(4,5,6));
            CHECK(s.getScale() == glm::vec3(2));
        }
        AND_THEN("rigidbody exists and can toggle gravity/static") {
            auto rb = s.getRigidBody();
            REQUIRE(rb != nullptr);
            rb->setUseGravity(true);
            rb->setStatic(false);
            CHECK(rb->isUsingGravity());
            CHECK_FALSE(rb->isStatic());
        }
    }

    GIVEN("a CubeEntity") {
        CubeEntity c(glm::vec3(1), glm::vec3(0));
        THEN("transform api works") {
            c.setPosition(glm::vec3(1,0,0));
            c.setScale(glm::vec3(3));
            CHECK(c.getPosition() == glm::vec3(1,0,0));
            CHECK(c.getScale() == glm::vec3(3));
        }
    }

    GIVEN("a PlaneEntity") {
        PlaneEntity p(glm::vec3(0,1,0), 1.0f);
        THEN("components may be null but api callable") {
            CHECK(p.getCollider() != nullptr);
            CHECK(p.getRigidBody() == nullptr);
        }
    }
}
