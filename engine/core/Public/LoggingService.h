#ifndef ARCHE_ENGINE_LOGGINGSERVICE_H
#define ARCHE_ENGINE_LOGGINGSERVICE_H

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

enum class LogLevel { INFO, WARNING, ERROR };

class ILogSink {
public:
    virtual void log(std::string_view message, LogLevel level) = 0;
    virtual ~ILogSink() = default;
};

/**
 * @brief LoggingService provides centralized logging for the engine.
 *        Subsystems and services should use this instead of global or static loggers.
 */
class LoggingService {
public:
    void addSink(std::unique_ptr<ILogSink> sink);
    void log(LogLevel level, const std::string& message, const char* file);

private:
    std::vector<std::unique_ptr<ILogSink>> sinks_;
    std::string formatLogMessage(LogLevel level, const std::string& message, const std::string& fileName);
};

class ScopedTimer {
public:
    ScopedTimer(std::shared_ptr<LoggingService> logger, const std::string& label);
    ~ScopedTimer();

private:
    std::shared_ptr<LoggingService> logger_;
    std::string label_;
    std::chrono::high_resolution_clock::time_point start_time_;
};

} // namespace Core
} // namespace Arche

#define ARCHE_LOG_INFO(logger, msg) (logger)->log(Arche::Core::LogLevel::INFO, (msg), __FILE__)
#define ARCHE_LOG_WARNING(logger, msg) (logger)->log(Arche::Core::LogLevel::WARNING, (msg), __FILE__)
#define ARCHE_LOG_ERROR(logger, msg) (logger)->log(Arche::Core::LogLevel::ERROR, (msg), __FILE__)

#endif // ARCHE_ENGINE_LOGGINGSERVICE_H