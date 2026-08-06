#include "../inc/Thread_safe_queue.hpp"

void ThreadSafeQueue::push(const std::string& message, LogLevel log_level) {
    {
        std::unique_lock<std::mutex> lock(mtx);
        cvNotFull.wait(lock, [this] { return logQueue.size() < kMaxSize || shutdown; });
        if (shutdown) return;
        logQueue.emplace(message, log_level);
    }
    cvNotEmpty.notify_one();
}

std::optional<std::pair<std::string, LogLevel>> ThreadSafeQueue::pop() {
    std::unique_lock<std::mutex> lock(mtx);
    cvNotEmpty.wait(lock, [this] { return !logQueue.empty() || shutdown; });
    if (logQueue.empty()) return std::nullopt;
    auto logEntry = logQueue.front();
    logQueue.pop();
    lock.unlock();
    cvNotFull.notify_one();
    return logEntry;
}

void ThreadSafeQueue::setShutdown() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        shutdown = true;
    }
    cvNotEmpty.notify_all();
    cvNotFull.notify_all();
}

size_t ThreadSafeQueue::getSize() const {
    std::lock_guard<std::mutex> lock(mtx);
    return logQueue.size();
}

bool ThreadSafeQueue::isEmpty() const {
    std::lock_guard<std::mutex> lock(mtx);
    return logQueue.empty();
}
    