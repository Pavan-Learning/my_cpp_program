#include "../inc/Logger_thread.hpp"

LoggerThread::LoggerThread(ThreadSafeQueue& queue, LegacyLogger& logger)
    : logQueue(queue), legacyLogger(logger) {}

LoggerThread::~LoggerThread() {
    stop();
}

void LoggerThread::start() {
    workerThread = std::thread(&LoggerThread::processLogs, this);
}

void LoggerThread::stop() {
    if (stopped) return;
    stopped = true;
    logQueue.setShutdown();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void LoggerThread::waitToFinishFlush() {
    std::unique_lock<std::mutex> lock(mtx);
    cvFlush.wait(lock, [this] { return logQueue.isEmpty() && !busy; });
}

void LoggerThread::processLogs() {
    while (auto logEntry = logQueue.pop()) {
        const auto& [message, log_level] = *logEntry;
        {
            std::lock_guard<std::mutex> lock(mtx);
            busy = true;
        }
        legacyLogger.writeLog(log_level, message);
        {
            std::lock_guard<std::mutex> lock(mtx);
            busy = false;
        }
        if (logQueue.isEmpty()) {
            cvFlush.notify_all();
        }
    }
}
