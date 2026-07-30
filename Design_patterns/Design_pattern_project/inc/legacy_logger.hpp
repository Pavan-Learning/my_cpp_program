#ifndef LEGACY_LOGGER_HPP
#define LEGACY_LOGGER_HPP
#include <iostream>
#include <string>
#include <ostream>
#include <fstream>
#include <thread>
#include <chrono>
#include "log_levels.hpp"

class LegacyLogger {
    private:
        static int logcount;
        LegacyLogger() = default;
        LegacyLogger(const LegacyLogger&) = delete; 
        LegacyLogger& operator=(const LegacyLogger&) = delete;

public:
    static LegacyLogger& getInstance() {
        static LegacyLogger instance;
        return instance; 
    }

    void consoleLog(LogLevel log_level, const std::string& message) {
        logcount++;
        std::ostream& ostream = std::cout;
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
};

int LegacyLogger::logcount = 0; // Initialize the static member variable

#endif // LEGACY_LOGGER_HPP
