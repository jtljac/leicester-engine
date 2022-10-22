//
// Created by jacob on 22/10/22.
//

#include "Utils/Logger.h"

#include <stdio.h>
#include <stdarg.h>

std::string Logger::getLogLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::CRITICAL:
            return "CRITICAL";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::INFO:
            return "INFO";
    }

    return "";
}

void Logger::log(LogLevel level, const std::string& message) {
    printf("%s: %s", getLogLevelString(level).c_str(), message.c_str());
}

void Logger::format(LogLevel level, std::string message, ...) { // NOLINT(performance-unnecessary-value-param)
    std::string levelString = getLogLevelString(level);

    va_list args;
    va_start(args, message);
    vprintf((levelString + ": " + message).c_str(), args);
    va_end(args);
}

void Logger::critical(const std::string& message) {
    Logger::log(LogLevel::CRITICAL, message);
}

void Logger::error(const std::string& message) {
    Logger::log(LogLevel::ERROR, message);
}

void Logger::warn(const std::string& message) {
    Logger::log(LogLevel::WARNING, message);
}

void Logger::info(const std::string& message) {
    Logger::log(LogLevel::INFO, message);
}

