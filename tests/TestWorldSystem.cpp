#include <catch2/catch_test_macros.hpp>
#include <WorldSystem.h>
#include <SphereEntity.h>
#include <CubeEntity.h>
#include <PlaneEntity.h>
#include <LoggingService.h>
#include <TimingService.h>
#include <PhysicsSystem.h>
#include <glm/glm.hpp>
#include <memory>

using namespace Arche::Scene;

static std::shared_ptr<Arche::Core::LoggingService> makeLogger() { return std::make_shared<Arche::Core::LoggingService>(); }
static std::shared_ptr<Arche::Core::TimingService> makeTiming(std::shared_ptr<Arche::Core::LoggingService> l){ return std::make_shared<Arche::Core::TimingService>(l); }

SCENARIO("WorldSystem manages entities and forwards updates", "[worldsystem]") {
    auto logger = makeLogger();
    auto timing = makeTiming(logger);
    auto physics = std::make_shared<Arche::Physics::PhysicsSystem>(logger);
    auto world = std::make_shared<WorldSystem>(logger, timing);
    world->setPhysics(physics);

    GIVEN("an empty world system") {
        auto view = world->view();
        CHECK(view.bodies.empty());

        WHEN("entities are added") {
            auto e1 = std::make_shared<SphereEntity>(1.0f, glm::vec3(0));
            auto e2 = std::make_shared<CubeEntity>(glm::vec3(1), glm::vec3(0));
            auto id1 = world->addEntity(e1);
            auto id2 = world->addEntity(e2);

            REQUIRE(world->getPhysicsSystem()->getBodies().size() == world->view().bodies.size());

            THEN("they appear in the view and have ids") {
                auto v2 = world->view();
                CHECK(v2.bodies.size() == 2);
                CHECK(e1->getID() == id1);
                CHECK(e2->getID() == id2);
            }

            AND_WHEN("positions are updated via API") {
                world->updateEntityPosition(id1, glm::vec3(1,2,3));
                THEN("entity reflects the new position") {
                    CHECK(e1->getPosition() == glm::vec3(1,2,3));
                }
            }
        }
    }
}
