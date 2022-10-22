//
// Created by jacob on 22/10/22.
//

#pragma once

#include <string>

enum class LogLevel {
    CRITICAL,
    ERROR,
    WARNING,
    INFO
};

class Logger {
    /**
     * Get a string representing the passed log level
     * @param level The log level
     * @return A string representing the log level
     */
    static std::string getLogLevelString(LogLevel level);
public:
    /**
     * Print a message to the console
     * @param level The severity of the message
     * @param message The message being printed to the console
     */
    static void log(LogLevel level, const std::string& message);

    /**
     * Print a message to the console with printf style formatting
     * @param level The severity of the message
     * @param message The format string being printed to the console
     * @param ... The parameters for the format string
     */
    static void format(LogLevel level, std::string message, ...);

    /**
     * Log a critical message to the console
     * @param message The message being logged
     */
    static void critical(const std::string& message);

    /**
     * Log an error message to the console
     * @param message The message being logged
     */
    static void error(const std::string& message);

    /**
     * Log a warn message to the console
     * @param message The message being logged
     */
    static void warn(const std::string& message);

    /**
     * Log an informational message to the console
     * @param message The message being logged
     */
    static void info(const std::string& message);
};

