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
};

struct TaskResult
{
    unsigned int taskId;
    bool result;
};



struct TaskItem
{
    TaskMetaData metadata;
    std::unique_ptr<ITask> task;
};
#endif