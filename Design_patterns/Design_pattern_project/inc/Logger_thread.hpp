
#ifndef LOGGER_THREAD_HPP
#define LOGGER_THREAD_HPP

#include "legacy_logger.hpp"
#include "Thread_safe_queue.hpp"

class LoggerThread {
private:
    std::thread workerThread;
    std::condition_variable cvFlush;
    std::mutex mtx;
    std::atomic<bool> busy{false};
    std::atomic<bool> stopped{false};
    ThreadSafeQueue& logQueue;
    LegacyLogger& legacyLogger;

public:
    LoggerThread(ThreadSafeQueue& queue, LegacyLogger& logger);
    ~LoggerThread();

    void start();
    void stop();
    void waitToFinishFlush();

private:
    void processLogs();
};

#endif // LOGGER_THREAD_HPP