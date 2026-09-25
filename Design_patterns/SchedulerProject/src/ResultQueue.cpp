#include "../inc/ResultQueue.hpp"

ResultQueue::~ResultQueue()
{
    resultQueueShutdown();
}

void ResultQueue::push(TaskResult&& queueResultTask)
{
    std::unique_lock<std::mutex> lock(mtx);
    if(resultQueueShutdownFlag)
        return;
    resultQueueStore.emplace(std::move(queueResultTask));
    lock.unlock();
    resultNotEmptyCV.notify_one();
}

std::optional<TaskResult> ResultQueue::pop()
{
    std::unique_lock<std::mutex> lock(mtx);
    resultNotEmptyCV.wait(lock, [this]{return !resultQueueStore.empty() || resultQueueShutdownFlag;});
    if(resultQueueStore.empty())
        return std::nullopt;
    auto resultTask = std::move(resultQueueStore.front());
    resultQueueStore.pop();
    lock.unlock();
    return resultTask;
}

void ResultQueue::resultQueueShutdown()
{
    std::unique_lock<std::mutex> lock(mtx);
    resultQueueShutdownFlag = true;
    lock.unlock();
    resultNotEmptyCV.notify_all();   
}
