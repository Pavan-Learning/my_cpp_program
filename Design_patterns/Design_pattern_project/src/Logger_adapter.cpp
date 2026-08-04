// this will implemet logger adapter class which will adapt the legacy logger to the new logger interface

#include "../inc/Logger_adapter.hpp"

LoggerAdapter::LoggerAdapter() : loggingThread(logQueue, LegacyLogger::getInstance()) {
    loggingThread.start();
}

LoggerAdapter::~LoggerAdapter() {
    shutdown();
}

void LoggerAdapter::log(LogLevel log_level, const std::string& message) {
    logQueue.push(message, log_level);
}

void LoggerAdapter::flush() {
    // this is not complete implementation, but for demonstration purposes, we can just flush the console output
    std::cout << std::flush; // Flush the console output
}

void LoggerAdapter::shutdown() {
    loggingThread.stop(); // Stop the logging thread
}