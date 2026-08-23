//

#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <bits/stdc++.h>
#include "TaskItem.hpp"
#

class ThreadSafeQueue
{
public:
    explicit ThreadSafeQueue(size_t maxSize);
    ~ThreadSafeQueue();
    void push(TaskItem&& task);
    std::optional<TaskItem> pop();
    bool isEmpty() const;
    size_t getSize() const;
    void shutdown();
    // bool isShutdown() const;

private:
    mutable std::mutex mtx;
    std::condition_variable cvNotEmpty;
    std::condition_variable cvNotFull;
    std::queue<TaskItem> taskQueue;
    bool shutdownFlag = false;
    size_t kMaxSize = 100; // Default maximum size of the queue
};
#endif
