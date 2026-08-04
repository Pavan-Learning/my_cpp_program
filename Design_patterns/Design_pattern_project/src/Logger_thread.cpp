// it have logger thread implementation, which will run in a separate thread and process log messages from the thread-safe queue.

#include "../inc/Logger_thread.hpp"

LoggerThread::LoggerThread(ThreadSafeQueue& queue, LegacyLogger& logger)
    : logQueue(queue), legacyLogger(logger) {
}

LoggerThread::~LoggerThread() {
    stop(); // Ensure the thread is stopped when the LoggerThread object is destroyed
}

void LoggerThread::start() {
    workerThread = std::thread(&LoggerThread::processLogs, this);
}

void LoggerThread::stop() {
    logQueue.setShutdown(); // Signal the queue to shutdown
    if (workerThread.joinable()) {
        workerThread.join(); // Wait for the thread to finish
    }
}

void LoggerThread::processLogs() {
    while (true) {
        auto logEntry = logQueue.pop();
        if (logEntry) {
            const auto& [message, log_level] = *logEntry;
            legacyLogger.writeLog(log_level, message);
        } else {
            break; // Exit the loop if the queue is empty and shutdown is signaled
        }
    }
}
