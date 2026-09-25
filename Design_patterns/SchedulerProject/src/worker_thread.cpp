// 

#include "../inc/worker_thread.hpp"

WorkerThread::WorkerThread(ThreadSafeQueue& queue, ResultQueue& resultQueue_) : safequeue(queue), resultQueue(resultQueue_){}

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
            if(!Result){
                TaskResult res{std::move(taskItem->metadata), Result, std::move(taskItem->task)};
                resultQueue.push(std::move(res));
            } 
        }
        else{
            break;
        }

    }
}
