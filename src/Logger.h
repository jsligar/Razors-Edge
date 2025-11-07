#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <stdarg.h>

// Log levels
enum LogLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
};

// Default log level (can be overridden by build flags)
#ifndef DEFAULT_LOG_LEVEL
    #ifdef DEBUG
        #define DEFAULT_LOG_LEVEL LOG_LEVEL_DEBUG
    #else
        #define DEFAULT_LOG_LEVEL LOG_LEVEL_INFO
    #endif
#endif

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogLevel(LogLevel level) {
        currentLogLevel = level;
    }

    LogLevel getLogLevel() const {
        return currentLogLevel;
    }

    void enableTimestamps(bool enable) {
        showTimestamps = enable;
    }

    void enableColors(bool enable) {
        showColors = enable;
    }

    // Core logging functions
    void log(LogLevel level, const char* tag, const char* format, ...) {
        if (level > currentLogLevel) return;

        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        printLog(level, tag, buffer);
    }

    void error(const char* tag, const char* format, ...) {
        if (LOG_LEVEL_ERROR > currentLogLevel) return;

        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        printLog(LOG_LEVEL_ERROR, tag, buffer);
    }

    void warn(const char* tag, const char* format, ...) {
        if (LOG_LEVEL_WARN > currentLogLevel) return;

        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        printLog(LOG_LEVEL_WARN, tag, buffer);
    }

    void info(const char* tag, const char* format, ...) {
        if (LOG_LEVEL_INFO > currentLogLevel) return;

        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        printLog(LOG_LEVEL_INFO, tag, buffer);
    }

    void debug(const char* tag, const char* format, ...) {
        if (LOG_LEVEL_DEBUG > currentLogLevel) return;

        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        printLog(LOG_LEVEL_DEBUG, tag, buffer);
    }

    void trace(const char* tag, const char* format, ...) {
        if (LOG_LEVEL_TRACE > currentLogLevel) return;

        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        printLog(LOG_LEVEL_TRACE, tag, buffer);
    }

private:
    Logger() : currentLogLevel(DEFAULT_LOG_LEVEL), showTimestamps(true), showColors(false) {}
    ~Logger() {}

    // Prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel currentLogLevel;
    bool showTimestamps;
    bool showColors;

    void printLog(LogLevel level, const char* tag, const char* message) {
        // Print timestamp if enabled
        if (showTimestamps) {
            Serial.printf("[%10lu] ", millis());
        }

        // Print color code if enabled
        if (showColors) {
            Serial.print(getColorCode(level));
        }

        // Print level
        Serial.print("[");
        Serial.print(getLevelString(level));
        Serial.print("] ");

        // Print tag
        Serial.print("[");
        Serial.print(tag);
        Serial.print("] ");

        // Print message
        Serial.println(message);

        // Reset color if enabled
        if (showColors) {
            Serial.print("\033[0m");
        }
    }

    const char* getLevelString(LogLevel level) {
        switch (level) {
            case LOG_LEVEL_ERROR: return "ERROR";
            case LOG_LEVEL_WARN:  return "WARN ";
            case LOG_LEVEL_INFO:  return "INFO ";
            case LOG_LEVEL_DEBUG: return "DEBUG";
            case LOG_LEVEL_TRACE: return "TRACE";
            default: return "UNKN ";
        }
    }

    const char* getColorCode(LogLevel level) {
        switch (level) {
            case LOG_LEVEL_ERROR: return "\033[31m"; // Red
            case LOG_LEVEL_WARN:  return "\033[33m"; // Yellow
            case LOG_LEVEL_INFO:  return "\033[32m"; // Green
            case LOG_LEVEL_DEBUG: return "\033[36m"; // Cyan
            case LOG_LEVEL_TRACE: return "\033[37m"; // White
            default: return "";
        }
    }
};

// Global convenience macros
#define LOG_ERROR(tag, ...) Logger::getInstance().error(tag, __VA_ARGS__)
#define LOG_WARN(tag, ...)  Logger::getInstance().warn(tag, __VA_ARGS__)
#define LOG_INFO(tag, ...)  Logger::getInstance().info(tag, __VA_ARGS__)
#define LOG_DEBUG(tag, ...) Logger::getInstance().debug(tag, __VA_ARGS__)
#define LOG_TRACE(tag, ...) Logger::getInstance().trace(tag, __VA_ARGS__)

#endif // LOGGER_H
