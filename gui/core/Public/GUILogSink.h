
#pragma once

#include <Diagnostics.h>
#include <mutex>
#include <queue>
#include <string>

namespace Arche {
    namespace GUI {
        class GUILogSink : public Arche::Core::ILogSink {
          public:
            void log(const std::string &message) override {
                // Implement GUI-specific logging here
                // For example, append the message to a text area in the GUI

                std::lock_guard<std::mutex> lock(loggedMessagesMutext);
                if (loggedMessages.size() >= maxLoggedMessages) {
                    loggedMessages.pop_front();
                }

                loggedMessages.push_back(message);
            }

            std::deque<std::string> getLoggedMessages() {
                std::lock_guard<std::mutex> lock(loggedMessagesMutext);
                return loggedMessages;
            }

          private:
            std::mutex loggedMessagesMutext;
            std::deque<std::string> loggedMessages;
            static constexpr size_t maxLoggedMessages = 100;
        };
    } // namespace GUI
} // namespace Arche