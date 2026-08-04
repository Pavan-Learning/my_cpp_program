

#ifndef LOGGER_THREAD_HPP
#define LOGGER_THREAD_HPP

#include "legacy_logger.hpp"
#include "Thread_safe_queue.hpp"

class LoggerThread {
    private:
        std::thread workerThread;
        ThreadSafeQueue& logQueue; // Reference to the shared log queue
        LegacyLogger& legacyLogger; // Reference to the legacy logger instance
    public:
        LoggerThread(ThreadSafeQueue& queue, LegacyLogger& logger);
        ~LoggerThread();

        void start();

        void stop();

    private:
        void processLogs();
};

#endif // LOGGER_THREAD_HPP