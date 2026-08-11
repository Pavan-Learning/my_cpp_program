#ifndef LEGACY_LOGGER_HPP
#define LEGACY_LOGGER_HPP

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include "log_levels.hpp"

class LegacyLogger {
private:
    std::atomic<int> logcount{0};
    LegacyLogger() = default;
    LegacyLogger(const LegacyLogger&) = delete;
    LegacyLogger& operator=(const LegacyLogger&) = delete;
    LegacyLogger(LegacyLogger&&) = delete; // Delete move constructor
    LegacyLogger& operator=(LegacyLogger&&) = delete; // Delete move assignment operator

public:
    static LegacyLogger& getInstance();
    void writeLog(LogLevel log_level, const std::string& message);
};

#endif // LEGACY_LOGGER_HPP
