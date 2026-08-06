#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <string>
#include <optional>
#include <condition_variable>

#include "log_levels.hpp"

class ThreadSafeQueue {
private:
    static constexpr size_t kMaxSize = 100;
    std::queue<std::pair<std::string, LogLevel>> logQueue;
    mutable std::mutex mtx;
    std::condition_variable cvNotEmpty;
    std::condition_variable cvNotFull;
    bool shutdown = false;

public:
    void push(const std::string& message, LogLevel log_level);
    std::optional<std::pair<std::string, LogLevel>> pop();
    size_t getSize() const;
    bool isEmpty() const;
    void setShutdown();
};

#endif // THREAD_SAFE_QUEUE_HPP