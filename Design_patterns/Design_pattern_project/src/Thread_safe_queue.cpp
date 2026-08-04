// this have thread safe queue implementation, which is used in the logger adapter to hold log messages before they are processed by the logging thread. The queue uses a mutex and condition variable to ensure that multiple threads can safely push and pop messages from the queue without causing

#include "../inc/Thread_safe_queue.hpp"

void ThreadSafeQueue::push(const std::string& message, LogLevel log_level) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        if(!shutdown) {
            if(logQueue.size() >= 100) { 
                logQueue.pop(); // Remove the oldest log entry if the queue is full
            }
            logQueue.emplace(std::make_pair(message, log_level));
        }
    }
    cv.notify_one();
}

std::optional<std::pair<std::string, LogLevel>> ThreadSafeQueue::pop() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return !logQueue.empty() || shutdown; });
    if (!logQueue.empty()) {
        auto logEntry = logQueue.front();
        logQueue.pop();
        return logEntry;
    }
    return std::nullopt; // Return an empty optional if the queue is empty
}

void ThreadSafeQueue::setShutdown() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        shutdown = true;
    }
    cv.notify_all();
}

size_t ThreadSafeQueue::getSize() const {
    const std::lock_guard<std::mutex> lock(mtx);
    return logQueue.size();
}

bool ThreadSafeQueue::isEmpty() const {
    const std::lock_guard<std::mutex> lock(mtx);
    return logQueue.empty();
}



    