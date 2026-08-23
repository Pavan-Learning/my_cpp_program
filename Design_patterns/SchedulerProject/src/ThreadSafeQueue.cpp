#include "../inc/ThreadSafeQueue.hpp"

ThreadSafeQueue::ThreadSafeQueue(size_t maxSize) : kMaxSize(maxSize) {}

ThreadSafeQueue::~ThreadSafeQueue() {
    shutdown();
}

void ThreadSafeQueue::push(TaskItem&& task)
{
    std::unique_lock<std::mutex> lock(mtx);
    cvNotFull.wait(lock, [this] { return taskQueue.size() < kMaxSize || shutdownFlag; });
    if (shutdownFlag) return;
    taskQueue.emplace(std::move(task));
    cvNotEmpty.notify_one();
}

std::optional<TaskItem> ThreadSafeQueue::pop()
{
    std::unique_lock<std::mutex> lock(mtx);
    cvNotEmpty.wait(lock, [this]{return !taskQueue.empty() || shutdownFlag;});
    if(taskQueue.empty())
    {
        return std::nullopt;
    }
    auto task = std::move(taskQueue.front());
    taskQueue.pop();
    lock.unlock();
    cvNotFull.notify_one();
    return task;
}

bool ThreadSafeQueue::isEmpty() const
{
    std::unique_lock<std::mutex> lock(mtx);
    return taskQueue.empty();
}

size_t ThreadSafeQueue::getSize() const
{
    std::unique_lock<std::mutex> lock(mtx);
    return taskQueue.size();
}

void ThreadSafeQueue::shutdown() {
    std::unique_lock<std::mutex> lock(mtx);
    shutdownFlag = true;
    lock.unlock();
    cvNotEmpty.notify_all();
    cvNotFull.notify_all();
}