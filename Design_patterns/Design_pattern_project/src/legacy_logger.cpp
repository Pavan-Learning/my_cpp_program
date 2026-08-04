// this have leggacy logger implementation, but for demonstration purposes, we can just flush the console output

#include "../inc/legacy_logger.hpp"

LegacyLogger& LegacyLogger::getInstance() {
    static LegacyLogger instance;
    return instance; 
}

void LegacyLogger::writeLog(LogLevel log_level, const std::string& message) {
    ++logcount;
    std::ostream& ostream = std::cout; // For demonstration, we are using console output. In a real scenario, this could be a file stream.
    if(log_level == LogLevel::ERROR || log_level == LogLevel::CRITICAL) {
        ostream << "\033[1;31m" << "this is error log" << " and log count " << logcount << " "  << message << "\033[0m" << std::endl;
    }
    else if(log_level == LogLevel::WARNING) {
        ostream << "\033[1;33m" << "this is warning log" << " and log count " << logcount << " " << message << "\033[0m" << std::endl;
    }
    else if(log_level == LogLevel::INFO) {
        ostream << "\033[1;32m" << "this is info log" << " and log count " << logcount << " " << message << "\033[0m" << std::endl;
    }
    else if(log_level == LogLevel::DEBUG) {
        ostream << "\033[1;34m" << "this is debug log" << " and log count " << logcount << " " << message << "\033[0m" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Simulate some delay for logging
}