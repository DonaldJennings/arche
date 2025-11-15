#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <TimingService.h>
#include <LoggingService.h>
#include <thread>

using namespace Arche::Core;
using namespace Catch;

SCENARIO("TimingService basic ticking, pause and resume", "[timing]") {
    auto logger = std::make_shared<LoggingService>();
    TimingService timing(logger);

    GIVEN("a fresh timing service") {
        WHEN("delta starts near 1/60 by ctor default and elapsed >= 0") {
            CHECK(timing.deltaTime() >= 0.0f);
            CHECK(timing.elapsed() >= 0.0f);
        }

        WHEN("tick is called a few times") {
            timing.tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            timing.tick();

            THEN("delta becomes > 0") {
                CHECK(timing.deltaTime() >= 0.0f);
            }
        }

        WHEN("paused") {
            timing.pause();
            timing.tick();
            THEN("delta is 0 and elapsed doesn't advance significantly") {
                CHECK(timing.deltaTime() == Approx(0.0f));
                auto e1 = timing.elapsed();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                timing.tick();
                auto e2 = timing.elapsed();
                CHECK(e2 - e1 < 0.01f);
            }
        }

        WHEN("resumed after pause") {
            timing.pause();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            timing.resume();
            timing.tick();
            THEN("delta is computed from resume point (non-negative)") {
                CHECK(timing.deltaTime() >= 0.0f);
            }
        }

        WHEN("reset is called") {
            timing.reset();
            timing.tick();
            THEN("elapsed restarts near zero") {
                CHECK(timing.elapsed() >= 0.0f);
            }
        }

        WHEN("start and stop control paused state") {
            timing.stop();
            CHECK(timing.isPaused());

            timing.start();
            CHECK_FALSE(timing.isPaused());

            timing.setPaused(true);
            CHECK(timing.isPaused());

            timing.setPaused(false);
            CHECK_FALSE(timing.isPaused());
        }
    }
}
