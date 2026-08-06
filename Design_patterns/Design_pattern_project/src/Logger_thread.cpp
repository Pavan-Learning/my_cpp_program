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
    std::cout << "Stopping logger thread...\n";
    logQueue.setShutdown(); // Signal the queue to shutdown
    if (workerThread.joinable()) {
        workerThread.join(); // Wait for the thread to finish
    }
}

void LoggerThread::waitToFinishFlush() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return logQueue.isEmpty() && !workerthreadbusy; });
    return; // Return when the queue is empty and the worker thread is not running
}

void LoggerThread::processLogs() {
    while(true)
    {
         std::cout << "Worker started\n";
        // Pop a log entry from the queue. If the queue is empty and shutdown is signaled, exit the loop.
        auto logEntry = logQueue.pop();
        if(!logEntry.has_value()) {
            // If the queue is empty and shutdown is signaled, exit the loop
            break;
        }
        // Process the log entry using the legacy logger and mark the worker thread as busy while processing
        const auto& [message, log_level] = *logEntry;
        {
            std::unique_lock<std::mutex> lock(mtx);
            workerthreadbusy = true;
        }
        legacyLogger.writeLog(log_level, message);
        std::cout << "Reached exit block\n";
        {
            std::unique_lock<std::mutex> lock(mtx);
            workerthreadbusy = false;
            if(logQueue.isEmpty()) {
                std::cout << "workerThreadRunning = false\n";
                cv.notify_all();
            }
        }
        std::cout << "Worker finished\n";
    }
}
