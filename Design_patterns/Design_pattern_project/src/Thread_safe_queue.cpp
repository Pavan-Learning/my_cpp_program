// this have thread safe queue implementation, which is used in the logger adapter to hold log messages before they are processed by the logging thread. The queue uses a mutex and condition variable to ensure that multiple threads can safely push and pop messages from the queue without causing

#include "../inc/Thread_safe_queue.hpp"

void ThreadSafeQueue::push(const std::string& message, LogLevel log_level) {
    {
        std::unique_lock<std::mutex> lock(mtx);
        if(!shutdown) {
            // here we added waiting the queue to be less than 100, to avoid memory overflow, 
            // if the queue is full, the thread will wait until there is space in the queue
            cv.wait(lock, [this] { return logQueue.size() < 100 || shutdown; });
            if(!shutdown) {
                logQueue.emplace(std::make_pair(message, log_level));
            }
            
            /* 
            // this is an alternative approach, where we remove the oldest log entry if the queue
            // is full, to make space for the new one. This way we avoid blocking the thread, but 
            // we lose some log entries.
            if(logQueue.size() >= 100) {
                logQueue.pop(); // Remove the oldest log entry to make space for the new one
            }
             logQueue.emplace(std::make_pair(message, log_level));
            */
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
        lock.unlock();
        // this will notify to waiting threads that there is space in the queue, so they can 
        // push new log entries. This is important to avoid deadlocks and ensure that the 
        // logging system can continue to function smoothly.
        cv.notify_one();
        return logEntry;
    }
    return std::nullopt;
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
    