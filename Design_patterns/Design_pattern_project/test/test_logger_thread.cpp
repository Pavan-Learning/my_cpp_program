#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include "../inc/Logger_thread.hpp"

class LoggerThreadTest : public ::testing::Test {
protected:
    ThreadSafeQueue queue;
    LegacyLogger& logger = LegacyLogger::getInstance();
};

TEST_F(LoggerThreadTest, LifecycleAndDoubleStop) {
    {
        LoggerThread lt(queue, logger);
        lt.start();
        queue.push("msg", LogLevel::INFO);
        lt.stop();
        lt.stop(); // Double stop must be safe
    }
    // Destructor after explicit stop must not crash
    SUCCEED();
}

TEST_F(LoggerThreadTest, FlushDrainsQueueInAllScenarios) {
    LoggerThread lt(queue, logger);
    lt.start();

    // Flush on empty queue — should return immediately
    lt.waitToFinishFlush();
    EXPECT_TRUE(queue.isEmpty());

    // Flush after pushing a batch
    for (int i = 0; i < 10; ++i) {
        queue.push("batch1_" + std::to_string(i), LogLevel::INFO);
    }
    lt.waitToFinishFlush();
    EXPECT_TRUE(queue.isEmpty());

    // Second flush cycle with mixed levels
    queue.push("b2", LogLevel::WARNING);
    queue.push("b3", LogLevel::ERROR);
    lt.waitToFinishFlush();
    EXPECT_TRUE(queue.isEmpty());

    // Flush after concurrent producer finishes
    std::thread producer([&]() {
        for (int i = 0; i < 20; ++i) {
            queue.push("concurrent_" + std::to_string(i), LogLevel::DEBUG);
        }
    });
    producer.join();
    lt.waitToFinishFlush();
    EXPECT_TRUE(queue.isEmpty());

    lt.stop();
}
