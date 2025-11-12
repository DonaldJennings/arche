#include <catch2/catch_test_macros.hpp>
#include <Renderer.h>
#include <LoggingService.h>

SCENARIO("Renderer basic configuration behaves predictably without a GL context", "[renderer]") {
    using namespace Arche::Render;
    auto logger = std::make_shared<Arche::Core::LoggingService>();
    Renderer r(logger, 320, 240, "test");

    GIVEN("a renderer") {
        THEN("viewport size setters/getters work") {
            CHECK(r.getWidth() == 320);
            CHECK(r.getHeight() == 240);
            r.setViewportSize(800, 600);
            CHECK(r.getWidth() == 800);
            CHECK(r.getHeight() == 600);
        }
    }
}
