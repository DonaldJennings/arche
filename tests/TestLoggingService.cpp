#include <catch2/catch_test_macros.hpp>
#include <LoggingService.h>
#include <memory>
#include <string>
#include <vector>

using namespace Arche::Core;

namespace {
    struct CapturingSink : ILogSink {
        struct Entry { std::string msg; LogLevel lvl; };
        std::vector<Entry> entries;
        void log(std::string_view message, LogLevel level) override {
            entries.push_back(Entry{std::string(message), level});
        }
    };
}

SCENARIO("LoggingService dispatches messages to sinks and preserves level", "[logging]") {
    GIVEN("a LoggingService with a capturing sink") {
        auto logger = std::make_shared<LoggingService>();
        auto sink = std::make_unique<CapturingSink>();
        auto* sinkPtr = sink.get();
        logger->addSink(std::move(sink));

        WHEN("multiple levels are logged") {
            logger->log(LogLevel::INFO, "hello info", __FILE__);
            logger->log(LogLevel::WARNING, "warn here", __FILE__);
            logger->log(LogLevel::ERROR, "boom", __FILE__);

            THEN("the sink receives them in order with correct levels and contents") {
                REQUIRE(sinkPtr->entries.size() == 3);
                CHECK(sinkPtr->entries[0].lvl == LogLevel::INFO);
                CHECK(sinkPtr->entries[1].lvl == LogLevel::WARNING);
                CHECK(sinkPtr->entries[2].lvl == LogLevel::ERROR);
                CHECK(sinkPtr->entries[0].msg.find("hello info") != std::string::npos);
                CHECK(sinkPtr->entries[1].msg.find("warn here") != std::string::npos);
                CHECK(sinkPtr->entries[2].msg.find("boom") != std::string::npos);
            }
        }
    }
}
