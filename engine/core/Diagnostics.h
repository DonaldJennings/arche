#ifndef ARCHE_ENGINE_DIAGNOSTICS_H
#define ARCHE_ENGINE_DIAGNOSTICS_H

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace Arche {
    namespace Core {

        class ILogSink {
          public:
            virtual void log(std::string const &message) = 0;
            virtual ~ILogSink() = default;
        };

        class ConsoleSink : public ILogSink {
          public:
            void log(std::string const &message) override {
                std::cout << message << std::endl;
            }
        };

        // Logging with sinks
        class Logger {
          public:
            enum class LogLevel { INFO, WARNING, ERROR };

            inline void addSink(std::unique_ptr<ILogSink> sink) {
                sinks_.emplace_back(std::move(sink));
            }

            void log(LogLevel level, std::string const &message,
                     const char *file) {
                std::string formatted{formatLogMessage(level, message, file)};
                for (std::unique_ptr<ILogSink> &sink : sinks_) {
                    sink->log(formatted);
                }
            }

          private:
            std::vector<std::unique_ptr<ILogSink>> sinks_;

            std::string formatLogMessage(Logger::LogLevel level,
                                         std::string const &message,
                                         std::string const &fileName) {
                auto now{std::chrono::system_clock::now()};
                auto time_t_now{std::chrono::system_clock::to_time_t(now)};
                std::tm tm_now;

#if defined(_MSC_VER)
                localtime_s(&tm_now, &time_t_now);
#else
                localtime_r(&time_t_now, &tm_now);
#endif
                std::ostringstream oss;
                oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");

                std::string levelStr;
                switch (level) {
                case Logger::LogLevel::INFO:
                    levelStr = "INFO";
                    break;
                case Logger::LogLevel::WARNING:
                    levelStr = "WARNING";
                    break;
                case Logger::LogLevel::ERROR:
                    levelStr = "ERROR";
                    break;
                default:
                    levelStr = "NULL";
                    break;
                }

                oss << " [" << levelStr << "] " << fileName << " - " << message;
                return oss.str();
            }
        };

        class ScopedTimer {
          public:
            ScopedTimer(std::shared_ptr<Logger> logger,
                        std::string const &label)
                : logger_(logger), label_(label),
                  start_time_(std::chrono::high_resolution_clock::now()) {}

            ~ScopedTimer() {
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - start_time_)
                        .count();
                if (logger_) {
                    logger_->log(Logger::LogLevel::INFO,
                                 label_ + " took " + std::to_string(duration) +
                                     " ms",
                                 __FILE__);
                }
            }

          private:
            std::shared_ptr<Logger> logger_;
            std::string label_;
            std::chrono::high_resolution_clock::time_point start_time_;
        };


    } // namespace Core
} // namespace Arche

#define ARCHE_LOG_INFO(logger, msg)                                            \
    (logger).log(Arche::Core::Logger::LogLevel::INFO, (msg), __FILE__)

#define ARCHE_LOG_WARNING(logger, msg)                                         \
    (logger).log(Arche::Core::Logger::LogLevel::WARNING, (msg), __FILE__)

#define ARCHE_LOG_ERROR(logger, msg)                                           \
    (logger).log(Arche::Core::Logger::LogLevel::ERROR, (msg), __FILE__)

#endif // ARCHE_ENGINE_DIAGNOSTICS_H