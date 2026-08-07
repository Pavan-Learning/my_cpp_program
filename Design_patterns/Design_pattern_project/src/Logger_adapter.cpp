#include "../inc/Logger_adapter.hpp"

LoggerAdapter::LoggerAdapter(size_t queueCapacity)
    : logQueue(queueCapacity),
      loggingThread(logQueue, LegacyLogger::getInstance()) {
    loggingThread.start();
}

LoggerAdapter::~LoggerAdapter() {
    shutdown();
}

void LoggerAdapter::log(LogLevel log_level, const std::string& message) {
    if (stopped) return;
    logQueue.push(message, log_level);
}

void LoggerAdapter::flush() {
    if (stopped) return;
    loggingThread.waitToFinishFlush();
}

void LoggerAdapter::shutdown() {
    if (stopped) return;
    stopped = true;
    loggingThread.stop();
}
