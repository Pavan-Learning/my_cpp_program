// this is a header file for the ITask interface

#ifndef ITASK_HPP
#define ITASK_HPP
#include <string>


class ITask {
    public:
        virtual ~ITask() = default;
        virtual bool execute() = 0; // Pure virtual function to be implemented by derived classes
        virtual std::string getTaskName() const = 0;
};

#endif // ITASK_HPP