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
    printf("%s: %s\n", getLogLevelString(level).c_str(), message.c_str());
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

