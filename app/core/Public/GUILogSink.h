#pragma once

#include <LoggingService.h>
#include <mutex>
#include <deque>
#include <string>
#include <string_view>
#include <chrono>

namespace Arche {
    namespace GUI {

        struct LogMessage
        {
            std::string text;
            Arche::Core::LogLevel level;
            std::chrono::system_clock::time_point timestamp;
        };

        class GUILogSink : public Arche::Core::ILogSink {
          public:
            void log(std::string_view message, Arche::Core::LogLevel level) override;
            std::deque<LogMessage> const& getLoggedMessages();

          private:
            std::mutex loggedMessagesMutex;
            std::deque<LogMessage> loggedMessages;
            static constexpr size_t maxLoggedMessages = 100;
        };
    } // namespace GUI
} // namespace Arche