#include "GUILogSink.h"

namespace Arche {
    namespace GUI {

        void GUILogSink::log(std::string_view message, Arche::Core::LogLevel level) {
            std::lock_guard<std::mutex> lock(loggedMessagesMutex);
            if (loggedMessages.size() >= maxLoggedMessages) {
                loggedMessages.pop_front();
            }

            loggedMessages.emplace_back(LogMessage{std::string(message), level, std::chrono::system_clock::now()});
        }

        std::deque<LogMessage> const& GUILogSink::getLoggedMessages() {
            std::lock_guard<std::mutex> lock(loggedMessagesMutex);
            return loggedMessages;
        }

    } // namespace GUI
} // namespace Arche