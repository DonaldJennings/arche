#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "core/FixedTimestepClock.h"

using Arche::Core::FixedTimestepClock;

TEST_CASE("FixedTimestepClock default construction", "[fixedtimestepclock]") {
    FixedTimestepClock clock;
    // The interpolation factor should be 0 at start
    REQUIRE(clock.interpolationFactor() == Catch::Approx(0.0f));
    // No step should be ready at start
    REQUIRE_FALSE(clock.stepReady());
}

TEST_CASE("FixedTimestepClock accumulate and stepReady", "[fixedtimestepclock]") {
    FixedTimestepClock clock(0.1f); // 0.1s timestep

    // Accumulate less than timestep
    clock.accumulate(0.05f);
    REQUIRE_FALSE(clock.stepReady());
    REQUIRE(clock.interpolationFactor() == Catch::Approx(0.5f));

    // Accumulate to reach the timestep
    clock.accumulate(0.05f);
    REQUIRE(clock.stepReady());
    REQUIRE(clock.interpolationFactor() == Catch::Approx(1.0f));

    // Accumulate more than one timestep
    clock.accumulate(0.2f);
    REQUIRE(clock.stepReady());
    REQUIRE(clock.interpolationFactor() == Catch::Approx(3.0f));
}

TEST_CASE("FixedTimestepClock consumeStep", "[fixedtimestepclock]") {
    FixedTimestepClock clock(0.1f);

    // Accumulate enough for two steps
    clock.accumulate(0.25f);
    REQUIRE(clock.stepReady());
    REQUIRE(clock.interpolationFactor() == Catch::Approx(2.5f));

    // Consume one step
    clock.consumeStep();
    REQUIRE(clock.stepReady());
    REQUIRE(clock.interpolationFactor() == Catch::Approx(1.5f));

    // Consume another step
    clock.consumeStep();
    REQUIRE_FALSE(clock.stepReady());
    REQUIRE(clock.interpolationFactor() == Catch::Approx(0.5f));

    // Consuming when not ready should not change accumulator
    clock.consumeStep();
    REQUIRE(clock.interpolationFactor() == Catch::Approx(0.5f));
}

TEST_CASE("FixedTimestepClock interpolationFactor with zero or negative timestep", "[fixedtimestepclock]") {
    FixedTimestepClock zeroClock(0.0f);
    zeroClock.accumulate(1.0f);
    REQUIRE(zeroClock.interpolationFactor() == Catch::Approx(0.0f));

    FixedTimestepClock negativeClock(-1.0f);
    negativeClock.accumulate(1.0f);
    REQUIRE(negativeClock.interpolationFactor() == Catch::Approx(0.0f));
}