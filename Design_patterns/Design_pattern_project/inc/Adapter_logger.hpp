// Here defined the Adapter_logger class, which is an adapter for the logging functionality. It allows the integration of different logging libraries into a unified interface, making it easier to switch between them without changing the core application code. The Adapter_logger class implements the ILogger interface, providing methods for logging messages at various levels (info, warning, error) and ensuring that the underlying logging library is used correctly.
#ifndef ADAPTER_LOGGER_HPP
#define ADAPTER_LOGGER_HPP

#include "logger_interface.hpp"
#include "log_levels.hpp"
#include "legacy_logger.hpp"
class AdapterLogger : public LoggerInterface {
    private:
        LegacyLogger& legacyLogger = LegacyLogger::getInstance();
public:
    AdapterLogger() = default;
    ~AdapterLogger() override = default;
    void log(LogLevel log_level, const std::string& message) override {
        legacyLogger.consoleLog(log_level, message); 
    }
    void flush() override {
       std::cout << std::flush; // Flush the console output
    }
    void shutdown() override {
        std::cout << "Shutting down the logger." << std::endl;
    }
};

#endif // ADAPTER_LOGGER_HPP