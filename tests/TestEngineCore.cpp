#include <catch2/catch_test_macros.hpp>
#include <EngineCore.h>

SCENARIO("EngineCore simulation state machine", "[core]") {
    using namespace Arche::Core;

    GIVEN("a freshly constructed EngineCore") {
        EngineCore engine;

        THEN("simulation starts in Idle state") {
            CHECK(engine.getSimulationState() == EngineCore::SimulationState::Idle);
            CHECK(engine.simulationIsPaused());
            CHECK_FALSE(engine.simulationIsRunning());
        }
    }
}
