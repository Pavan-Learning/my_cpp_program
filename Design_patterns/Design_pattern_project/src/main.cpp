#include "../inc/Logger_factory.hpp"
#include "../inc/Logger_adapter.hpp"
#include <thread>
#include <vector>

int main() {
    // LoggerAdapterFactory adapterFactory;
    // auto logger = adapterFactory.createLogger();
    // logger->log(LogLevel::INFO, "This is an info message.");
    // logger->log(LogLevel::ERROR, "This is an error message.");
    // logger->flush();
    // logger->shutdown();
    LoggerAdapter logging;
    LoggerAdapter TestLogger;
    logging.shutdown();
    TestLogger.shutdown();
    std::cout << "Logger shutdown completed\n";



    // test the logger flush implemetaion 
    // LoggerAdapter TestLogger;

    // TestLogger.shutdown();

    // auto start = std::chrono::steady_clock::now();
    // TestLogger.log(LogLevel::DEBUG, "This is a debug message.");
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // TestLogger.log(LogLevel::INFO, "This is an info message.");
    // TestLogger.log(LogLevel::WARNING, "This is a warning message.");

    // TestLogger.flush();

    // auto end = std::chrono::steady_clock::now();

    // std::cout << "Flush took "
    //         << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
    //         << " ms\n";
    // std::cout << "Done\n";


    // need to test with stress test, to see if the logger can handle high volume of log messages 
    // without losing any messages or crashing. We can create multiple threads that will log messages 
    // concurrently and see if the logger can handle it.

    /*LoggerAdapter stressTestLogger;
    const int numThreads = 2;
    const int messagesPerThread = 10;
    std::vector<std::thread> threads;

    for(int i=0; i<numThreads; ++i) {
        threads.emplace_back([&stressTestLogger, i]() {
            for(int j=0; j<messagesPerThread; ++j) {
                stressTestLogger.log(LogLevel::DEBUG, "Thread " + std::to_string(i) + " logging message " + std::to_string(j));
                stressTestLogger.log(LogLevel::INFO, "Thread " + std::to_string(i) + " logging message " + std::to_string(j));
                if(j % 2 == 0) {
                    stressTestLogger.flush(); // Flush logs every 2 messages
                }
            }
        });
    }


    for(auto& thread : threads) {
        thread.join();
    }

    stressTestLogger.flush(); // Flush any remaining logs

    stressTestLogger.shutdown();
    */
    return 0;
}



