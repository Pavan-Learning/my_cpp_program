#ifndef RESULT_QUEUE
#define RESULT_QUEUE

#include <bits/stdc++.h>
#include "TaskItem.hpp"

class ResultQueue {
    public:
        ResultQueue() = default;
        ~ResultQueue();
        void push(TaskResult&& queueResultTask);
        std::optional<TaskResult> pop();
        void resultQueueShutdown();

    private:
        std::mutex mtx;
        std::queue<TaskResult> resultQueueStore;
        std::condition_variable resultNotEmptyCV;
        bool resultQueueShutdownFlag = false;
};

#endif