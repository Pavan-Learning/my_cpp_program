#include "../inc/Logger_factory.hpp"
#include "../inc/Logger_adapter.hpp"
#include <thread>
#include <vector>

int main() {
    LoggerAdapterFactory adapterFactory;
    auto logger = adapterFactory.createLogger();
    logger->log(LogLevel::INFO, "This is an info message.");
    logger->log(LogLevel::ERROR, "This is an error message.");
    logger->flush();
    logger->shutdown();

    // need to test with stress test, to see if the logger can handle high volume of log messages 
    // without losing any messages or crashing. We can create multiple threads that will log messages 
    // concurrently and see if the logger can handle it.

    LoggerAdapter stressTestLogger;
    const int numThreads = 101;
    const int messagesPerThread = 1000;
    std::vector<std::thread> threads;

    for(int i=0; i<numThreads; ++i) {
        threads.emplace_back([&stressTestLogger, i]() {
            for(int j=0; j<messagesPerThread; ++j) {
                stressTestLogger.log(LogLevel::DEBUG, "Thread " + std::to_string(i) + " logging message " + std::to_string(j));
                stressTestLogger.log(LogLevel::INFO, "Thread " + std::to_string(i) + " logging message " + std::to_string(j));
                stressTestLogger.log(LogLevel::WARNING, "Thread " + std::to_string(i) + " logging message " + std::to_string(j));
                stressTestLogger.log(LogLevel::ERROR, "Thread " + std::to_string(i) + " logging message " + std::to_string(j));
            }
        });
    }


    for(auto& thread : threads) {
        thread.join();
    }

    stressTestLogger.shutdown();
    return 0;
}



