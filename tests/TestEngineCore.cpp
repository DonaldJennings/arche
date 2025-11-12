#include <catch2/catch_test_macros.hpp>
#include <EngineCore.h>
#include <LoggingService.h>

SCENARIO("EngineCore subsystem loop calls update on subsystems", "[core]") {
    using namespace Arche::Core;

    class DummySubsystem : public ISubsystem {
    public:
        int updates{0};
        void initialise() override {}
        void update(double) override { ++updates; }
        void shutdown() override {}
    };

    GIVEN("an EngineCore with a dummy subsystem") {
        auto engine = std::make_shared<EngineCore>();
        auto dummy = std::make_shared<DummySubsystem>();
        engine->registerSubsystem(dummy);

        WHEN("update is called") {
            engine->update();
            THEN("the subsystem update was invoked") {
                CHECK(dummy->updates == 1);
            }
        }
    }
}
