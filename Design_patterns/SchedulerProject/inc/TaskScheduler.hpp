// this is a header file for TaskScheduler class
#ifndef TASK_SCHEDULER_HPP
#define TASK_SCHEDULER_HPP

#include <bits/stdc++.h>
#include "TaskItem.hpp"
#include "ThreadSafeQueue.hpp"
#include "worker_thread.hpp"

class TaskScheduler
{
public:
    explicit TaskScheduler(size_t queueSize, size_t wokrerCount);
    ~TaskScheduler();

    void submitTask(std::unique_ptr<ITask> task, Priority priority);

private:

    TaskItem createTaskItem(std::unique_ptr<ITask> task, Priority priority);

    void start(size_t workerCount);
    void stop();

    ThreadSafeQueue taskQueue;
    ResultQueue resultsQueue;
    std::vector<std::unique_ptr<WorkerThread>> taskWorker;
    std::atomic<uint64_t> taskID{101};
    size_t queueSize;
    size_t workerCount;
};

#endif // TASK_SCHEDULER_HPP