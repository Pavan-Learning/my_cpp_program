// here we defined the Logger_factory class, which is responsible for creating instances of different logger implementations based on the specified logging library. The Logger_factory class provides a static method to create a logger instance, allowing the application to easily switch between different logging libraries without modifying the core code. This promotes flexibility and maintainability in the logging functionality of the application.
#ifndef LOGGER_FACTORY_HPP
#define LOGGER_FACTORY_HPP

#include "logger_interface.hpp"
#include "Adapter_logger.hpp"
#include "Adapter_logger.hpp"

class LoggerFactory {
public:
    virtual std::unique_ptr<LoggerInterface> createLogger() = 0; // Pure virtual function to create a logger instance
    virtual ~LoggerFactory() = default; // Virtual destructor for proper cleanup of derived classes
};

class AdapterLoggerFactory : public LoggerFactory {
public:
    std::unique_ptr<LoggerInterface> createLogger() override {
        return std::make_unique<AdapterLogger>(); // Create and return an instance of AdapterLogger
    }
};
        


#endif // LOGGER_FACTORY_HPP