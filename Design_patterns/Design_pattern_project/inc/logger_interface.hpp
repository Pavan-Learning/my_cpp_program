// This is interface for the logger class. It is used to log messages to different outputs (e.g., console, file, etc.) and can be implemented in various ways depending on the logging requirements.
#ifndef LOGGER_INTERFACE_HPP
#define LOGGER_INTERFACE_HPP

#include <iostream>
#include <string>
#include "log_levels.hpp"

class LoggerInterface {
public:
    virtual ~LoggerInterface() = default;
    virtual void log(LogLevel level, const std::string& message) = 0; // Pure virtual function
    virtual void flush() = 0; // Pure virtual function to flush the log output
    virtual void shutdown() = 0; // Pure virtual function to perform any necessary cleanup before shutting down the logger
};

#endif // LOGGER_INTERFACE_HPP
