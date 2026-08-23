// 

#include "../inc/worker_thread.hpp"

WorkerThread::WorkerThread(ThreadSafeQueue& queue) : safequeue(queue){}

WorkerThread::~WorkerThread()
{
    stop();
}

void WorkerThread::start()
{
    worker = std::thread(&WorkerThread::workerOperation, this);
}

void WorkerThread::stop()
{
    safequeue.shutdown();
    if(worker.joinable())
        worker.join();
}

void WorkerThread::workerOperation()
{
    while(true)
    {
        auto taskItem = safequeue.pop();
        if(taskItem != std::nullopt)
        {
            auto Result = taskItem->task->execute();
            TaskResult res{taskItem->metadata.taskId, Result};
            // here need to impleemt the ResultQueue things 
        }
        else{
            break;
        }

    }
}