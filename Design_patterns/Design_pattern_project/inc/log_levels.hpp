// this file have enumeration of log levels. It is used to specify the severity or importance of log messages in a logging system. Different log levels can be used to categorize log messages and control the verbosity of logging output.
#ifndef LOG_LEVELS_HPP
#define LOG_LEVELS_HPP

enum class LogLevel {
    DEBUG,   // Detailed information for debugging purposes
    INFO,    // General informational messages
    WARNING, // Indications of potential issues or important events
    ERROR,   // Error messages indicating failures or problems
    CRITICAL // Severe error messages indicating critical failures
};

#endif // LOG_LEVELS_HPP
