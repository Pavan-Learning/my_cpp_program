// Here defined the logger adapter class, 
#ifndef LOGGER_ADAPTER_HPP
#define LOGGER_ADAPTER_HPP

#include "logger_interface.hpp"
#include "log_levels.hpp"
#include "legacy_logger.hpp"
#include "Thread_safe_queue.hpp"
#include "Logger_thread.hpp"


class LoggerAdapter : public ILogger {
    private:
    ThreadSafeQueue logQueue; // Thread-safe queue to hold log messages
    LoggerThread loggingThread; // Thread for processing log messages
public:
    LoggerAdapter();
    ~LoggerAdapter();
    void log(LogLevel log_level, const std::string& message) override;
    void flush() override;
    void shutdown() override;
};

#endif // LOGGER_ADAPTER_HPP