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

/**
 * @brief Enumeration of log severity levels.
 * 
 * Defines the severity levels for log messages, from informational
 * to critical errors.
 */
enum class LogLevel { 
    INFO,     ///< Informational messages
    WARNING,  ///< Warning messages indicating potential issues
    ERROR     ///< Error messages indicating failures
};

/**
 * @brief Interface for log message sinks.
 * 
 * This interface must be implemented by any class that wants to receive
 * log messages from the logging service. Examples include console output,
 * file writers, or GUI log panels.
 */
class ILogSink {
public:
    /**
     * @brief Process a log message.
     * 
     * @param message The formatted log message to process
     * @param level The severity level of the message
     */
    virtual void log(std::string_view message, LogLevel level) = 0;
    
    /**
     * @brief Virtual destructor for proper cleanup.
     */
    virtual ~ILogSink() = default;
};

/**
 * @brief LoggingService provides centralized logging for the engine.
 * 
 * This service manages log message routing to multiple sinks and provides
 * a unified logging interface for all engine subsystems. Messages are
 * formatted with timestamps and file information before being dispatched
 * to all registered sinks.
 * 
 * @note Subsystems and services should use this instead of global or static loggers.
 */
class LoggingService {
public:
    /**
     * @brief Register a new log sink to receive messages.
     * 
     * Transfers ownership of the sink to the logging service. All future
     * log messages will be sent to this sink.
     * 
     * @param sink Unique pointer to the log sink implementation
     */
    void addSink(std::unique_ptr<ILogSink> sink);
    
    /**
     * @brief Log a message at the specified level.
     * 
     * Formats the message with timestamp and file information, then
     * dispatches it to all registered sinks.
     * 
     * @param level The severity level of the message
     * @param message The message content
     * @param file The source file name where the log was generated
     */
    void log(LogLevel level, const std::string& message, const char* file);

private:
    std::vector<std::unique_ptr<ILogSink>> sinks_;  ///< Registered log sinks
    
    /**
     * @brief Format a log message with metadata.
     * 
     * @param level The log level
     * @param message The message content
     * @param fileName The source file name
     * @return Formatted log message string
     */
    std::string formatLogMessage(LogLevel level, const std::string& message, const std::string& fileName);
};

/**
 * @brief RAII-style timer for performance measurement.
 * 
 * This class automatically logs the elapsed time between construction
 * and destruction, making it useful for measuring the duration of
 * operations or scope blocks.
 * 
 * @code
 * {
 *     ScopedTimer timer(logger, "MyOperation");
 *     // ... code to measure ...
 * } // Automatically logs elapsed time here
 * @endcode
 */
class ScopedTimer {
public:
    /**
     * @brief Construct a scoped timer and start timing.
     * 
     * @param logger Shared pointer to the logging service
     * @param label Descriptive label for the timed operation
     */
    ScopedTimer(std::shared_ptr<LoggingService> logger, const std::string& label);
    
    /**
     * @brief Destructor that logs the elapsed time.
     * 
     * Automatically calculates and logs the time elapsed since construction.
     */
    ~ScopedTimer();

private:
    std::shared_ptr<LoggingService> logger_;                            ///< Logger instance
    std::string label_;                                                  ///< Operation label
    std::chrono::high_resolution_clock::time_point start_time_;         ///< Start timestamp
};

} // namespace Core
} // namespace Arche

/**
 * @def ARCHE_LOG_INFO
 * @brief Macro for logging informational messages.
 * @param logger Pointer to LoggingService instance
 * @param msg Message string to log
 */
#define ARCHE_LOG_INFO(logger, msg) (logger)->log(Arche::Core::LogLevel::INFO, (msg), __FILE__)

/**
 * @def ARCHE_LOG_WARNING
 * @brief Macro for logging warning messages.
 * @param logger Pointer to LoggingService instance
 * @param msg Warning message string to log
 */
#define ARCHE_LOG_WARNING(logger, msg) (logger)->log(Arche::Core::LogLevel::WARNING, (msg), __FILE__)

/**
 * @def ARCHE_LOG_ERROR
 * @brief Macro for logging error messages.
 * @param logger Pointer to LoggingService instance
 * @param msg Error message string to log
 */
#define ARCHE_LOG_ERROR(logger, msg) (logger)->log(Arche::Core::LogLevel::ERROR, (msg), __FILE__)

#endif // ARCHE_ENGINE_LOGGINGSERVICE_H