//

#ifndef WORKER_THREAD_HPP
#define WORKER_THREAD_HPP

#include <bits/stdc++.h>
#include "ThreadSafeQueue.hpp"
#include "ResultQueue.hpp"

class WorkerThread
{
public:
    explicit WorkerThread(ThreadSafeQueue& queue, ResultQueue& resultQueue_);
    ~WorkerThread();
    void start();
    void stop();
    // bool isRunning() const;
private:
    void workerOperation();
    std::thread worker;
    ThreadSafeQueue& safequeue;
    ResultQueue& resultQueue;
};

#endif