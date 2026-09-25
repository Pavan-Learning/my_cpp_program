#ifndef TASKITEM
#define TASKITEM

#include <bits/stdc++.h>
#include "ITask.hpp"

enum class Priority
{
    LOW,
    MEDIUM,
    HIGH
};

struct TaskMetaData
{
    unsigned int taskId;
    Priority taskPriority;
    unsigned int attemptCount = 0;
};

struct TaskResult
{
    TaskMetaData metadata;
    bool result;
    std::unique_ptr<ITask> task;
};



struct TaskItem
{
    TaskMetaData metadata;
    std::unique_ptr<ITask> task;
};
#endif