#pragma once

#include <LoggingService.h>
#include <mutex>
#include <queue>
#include <string>

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
            void log(std::string_view message, Arche::Core::LogLevel level) override {
                // Implement GUI-specific logging here
                // For example, append the message to a text area in the GUI

                std::lock_guard<std::mutex> lock(loggedMessagesMutex);
                if (loggedMessages.size() >= maxLoggedMessages) {
                    loggedMessages.pop_front();
                }

                loggedMessages.emplace_back(LogMessage{std::string(message), level, std::chrono::system_clock::now()});
            }

            std::deque<LogMessage> const& getLoggedMessages() {
                std::lock_guard<std::mutex> lock(loggedMessagesMutex);
                return loggedMessages;
            }

          private:
            std::mutex loggedMessagesMutex;
            std::deque<LogMessage> loggedMessages;
            static constexpr size_t maxLoggedMessages = 100;
        };
    } // namespace GUI
} // namespace Arche