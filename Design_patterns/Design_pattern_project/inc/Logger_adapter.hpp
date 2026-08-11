#ifndef LOGGER_ADAPTER_HPP
#define LOGGER_ADAPTER_HPP

#include "logger_interface.hpp"
#include "Thread_safe_queue.hpp"
#include "Logger_thread.hpp"
#include "legacy_logger.hpp"

class LoggerAdapter : public ILogger {
private:
    ThreadSafeQueue logQueue;
    LoggerThread loggingThread;
    std::atomic<bool> stopped{false};

public:
    explicit LoggerAdapter(size_t queueCapacity = 100);
    ~LoggerAdapter() override;
    void log(LogLevel log_level, const std::string& message) override;
    void flush() override;
    void shutdown() override;
};

#endif // LOGGER_ADAPTER_HPP