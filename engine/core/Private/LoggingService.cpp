#include "LoggingService.h"

namespace Arche {
namespace Core {

void LoggingService::addSink(std::unique_ptr<ILogSink> sink) {
    sinks_.emplace_back(std::move(sink));
}

void LoggingService::log(LogLevel level, const std::string& message, const char* file) {
    std::string formatted = formatLogMessage(level, message, file);
    for (auto& sink : sinks_) {
        sink->log(formatted, level);
    }
}

std::string LoggingService::formatLogMessage(LogLevel level, const std::string& message, const std::string& fileName) {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
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
    case LogLevel::INFO:
        levelStr = "INFO";
        break;
    case LogLevel::WARNING:
        levelStr = "WARNING";
        break;
    case LogLevel::ERROR:
        levelStr = "ERROR";
        break;
    default:
        levelStr = "NULL";
        break;
    }

    oss << " [" << levelStr << "] " << fileName << " - " << message;
    return oss.str();
}

} // namespace Core
} // namespace Arche