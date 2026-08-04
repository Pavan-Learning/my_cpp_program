#ifndef LEGACY_LOGGER_HPP
#define LEGACY_LOGGER_HPP
#include <iostream>
#include <string>
#include <memory>
#include <ostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <sstream>
#include <atomic>
#include "log_levels.hpp"

class LegacyLogger {
    private:
        std::atomic<int> logcount{0}; // Atomic counter for log entries
        LegacyLogger() = default;
        LegacyLogger(const LegacyLogger&) = delete; 
        LegacyLogger& operator=(const LegacyLogger&) = delete;

public:
    static LegacyLogger& getInstance();

    void writeLog(LogLevel log_level, const std::string& message);
};

#endif // LEGACY_LOGGER_HPP
