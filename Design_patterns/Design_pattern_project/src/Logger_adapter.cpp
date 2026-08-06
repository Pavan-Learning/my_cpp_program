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
    std::cout << "Debug : ********** Flushing logs *****" << std::endl;
    loggingThread.waitToFinishFlush(); // Wait for the logging thread to finish processing logs
    
    std::cout << "Debug : ********** Flushing logs completed *****" << std::endl;
}

void LoggerAdapter::shutdown() {
    loggingThread.stop(); // Stop the logging thread
}