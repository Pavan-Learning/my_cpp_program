#include "../inc/TaskScheduler.hpp"

TaskScheduler::TaskScheduler(size_t queueSize, size_t workerCount)
: taskQueue(queueSize){
    start(workerCount);
}

TaskScheduler::~TaskScheduler()
{
    stop();
}

void TaskScheduler::submitTask(std::unique_ptr<ITask> task, Priority priority){
    auto taskData = createTaskItem(std::move(task), priority);
    taskQueue.push(std::move(taskData));
}

TaskItem TaskScheduler::createTaskItem(std::unique_ptr<ITask> task, Priority priority)
{
    TaskMetaData metadata{taskID, priority};
    TaskItem taskData{metadata, std::move(task)};
    taskID++;
    return taskData;
}

void TaskScheduler::start(size_t workerCount)
{
    for (size_t i = 0; i < workerCount; ++i)
    {
        auto worker = std::make_unique<WorkerThread>(taskQueue, resultsQueue);
        taskWorker.push_back(std::move(worker));
        taskWorker.back()->start();
    }
}

void TaskScheduler::stop()
{
    for(int i =0; i < taskWorker.size(); i++)
    {
        taskWorker[i]->stop();
    }
}



