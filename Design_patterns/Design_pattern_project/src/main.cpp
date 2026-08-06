#include "../inc/Logger_factory.hpp"
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

int main() {
    // Basic usage via factory
    LoggerAdapterFactory factory;
    auto logger = factory.createLogger();
    logger->log(LogLevel::INFO, "This is an info message.");
    logger->log(LogLevel::ERROR, "This is an error message.");
    logger->flush();
    logger->shutdown();

    // Stress test: multiple threads logging concurrently
    LoggerAdapter stressTestLogger;
    constexpr int numThreads = 10;
    constexpr int messagesPerThread = 10;
    std::vector<std::thread> threads;

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&stressTestLogger, i]() {
            for (int j = 0; j < messagesPerThread; ++j) {
                stressTestLogger.log(LogLevel::DEBUG, "Thread " + std::to_string(i) + " msg " + std::to_string(j));
                stressTestLogger.log(LogLevel::INFO, "Thread " + std::to_string(i) + " msg " + std::to_string(j));
                if (j % 5 == 0) {
                    stressTestLogger.flush();
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    stressTestLogger.flush();

    auto end = std::chrono::steady_clock::now();
    std::cout << "Stress test completed in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n";

    stressTestLogger.shutdown();
    return 0;
}
