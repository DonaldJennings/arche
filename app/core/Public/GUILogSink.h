#pragma once

#include <LoggingService.h>
#include <mutex>
#include <deque>
#include <string>
#include <string_view>
#include <chrono>

namespace Arche {
    namespace GUI {

        /**
         * @brief Log message entry for UI display.
         * 
         * Contains a log message along with its severity level and timestamp
         * for display in the editor log panel.
         */
        struct LogMessage
        {
            std::string text;                                       ///< Message content
            Arche::Core::LogLevel level;                           ///< Severity level
            std::chrono::system_clock::time_point timestamp;       ///< When logged
        };

        /**
         * @brief Log sink that captures messages for GUI display.
         * 
         * GUILogSink implements the ILogSink interface to receive log messages
         * from the logging service and stores them in a thread-safe queue for
         * display in the editor's log panel.
         * 
         * Messages are kept in a circular buffer with a maximum size to prevent
         * unbounded memory growth during long editor sessions.
         * 
         * @note This class is thread-safe and can receive logs from any thread.
         */
        class GUILogSink : public Arche::Core::ILogSink {
          public:
            /**
             * @brief Receive a log message.
             * 
             * Stores the message in the internal queue with a timestamp.
             * If the queue exceeds the maximum size, the oldest message is removed.
             * 
             * @param message Log message text
             * @param level Message severity level
             */
            void log(std::string_view message, Arche::Core::LogLevel level) override;
            
            /**
             * @brief Get all captured log messages.
             * 
             * Returns a const reference to the message queue for display.
             * Thread-safe for reading while messages may be added.
             * 
             * @return Const reference to the log message queue
             */
            std::deque<LogMessage> const& getLoggedMessages();

          private:
            std::mutex loggedMessagesMutex;                 ///< Protects message queue
            std::deque<LogMessage> loggedMessages;          ///< Circular message buffer
            static constexpr size_t maxLoggedMessages = 100;///< Maximum messages to keep
        };
    } // namespace GUI
} // namespace Arche