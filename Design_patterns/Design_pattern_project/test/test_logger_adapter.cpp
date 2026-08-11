#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include "../inc/Logger_adapter.hpp"
#include "../inc/Logger_factory.hpp"

// --- Unit Tests ---

TEST(LoggerAdapterTest, LogAllLevelsWithFlushCycles) {
    LoggerAdapter adapter;

    // Flush on empty — should not block
    adapter.flush();

    // Log all levels with multiple flush cycles
    adapter.log(LogLevel::DEBUG, "debug msg");
    adapter.log(LogLevel::INFO, "info msg");
    adapter.flush();

    adapter.log(LogLevel::WARNING, "warning msg");
    adapter.log(LogLevel::ERROR, "error msg");
    adapter.log(LogLevel::CRITICAL, "critical msg");
    adapter.flush();

    adapter.shutdown();
}

TEST(LoggerAdapterTest, PostShutdownBehavior) {
    LoggerAdapter adapter;
    adapter.log(LogLevel::INFO, "before shutdown");
    adapter.shutdown();

    // All of these must be silent no-ops
    adapter.log(LogLevel::INFO, "should be ignored");
    adapter.log(LogLevel::ERROR, "also ignored");
    adapter.flush();
    adapter.shutdown(); // Idempotent
    adapter.shutdown(); // Triple shutdown
}

TEST(LoggerAdapterTest, DestructorHandlesCleanup) {
    {
        LoggerAdapter adapter;
        adapter.log(LogLevel::INFO, "destructor test");
    }
    // Rapid create-destroy cycles
    for (int i = 0; i < 5; ++i) {
        LoggerAdapter adapter;
        adapter.log(LogLevel::INFO, "rapid_" + std::to_string(i));
        adapter.flush();
    }
    SUCCEED();
}

// --- Factory Tests ---

TEST(LoggerFactoryTest, CreatesIndependentInstances) {
    LoggerAdapterFactory factory;
    auto logger1 = factory.createLogger();
    auto logger2 = factory.createLogger();
    ASSERT_NE(logger1, nullptr);
    ASSERT_NE(logger2, nullptr);
    EXPECT_NE(logger1.get(), logger2.get());

    logger1->log(LogLevel::INFO, "from logger1");
    logger2->log(LogLevel::ERROR, "from logger2");
    logger1->flush();
    logger2->flush();
    logger1->shutdown();
    logger2->shutdown();
}

// --- Integration Tests ---

TEST(LoggerIntegrationTest, MultiThreadedLoggingWithFlush) {
    LoggerAdapter adapter;
    constexpr int numThreads = 8;
    constexpr int msgsPerThread = 25;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&adapter, i]() {
            for (int j = 0; j < msgsPerThread; ++j) {
                adapter.log(LogLevel::INFO, "T" + std::to_string(i) + "_" + std::to_string(j));
                if (j % 5 == 0) {
                    adapter.flush();
                }
            }
        });
    }

    for (auto& t : threads) t.join();
    adapter.flush();
    adapter.shutdown();
}

TEST(LoggerIntegrationTest, ConcurrentFlushAndLog) {
    LoggerAdapter adapter;

    std::thread producer([&]() {
        for (int i = 0; i < 50; ++i) {
            adapter.log(LogLevel::INFO, "msg_" + std::to_string(i));
        }
    });

    std::thread flusher([&]() {
        for (int i = 0; i < 10; ++i) {
            adapter.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    producer.join();
    flusher.join();
    adapter.flush();
    adapter.shutdown();
}

TEST(LoggerIntegrationTest, ShutdownWhileProducersActive) {
    LoggerAdapter adapter;
    std::atomic<bool> keepGoing{true};
    std::vector<std::thread> producers;

    for (int i = 0; i < 4; ++i) {
        producers.emplace_back([&adapter, &keepGoing, i]() {
            int seq = 0;
            while (keepGoing.load()) {
                adapter.log(LogLevel::DEBUG, "T" + std::to_string(i) + "_" + std::to_string(seq++));
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    keepGoing = false;
    for (auto& t : producers) t.join();
    adapter.shutdown();
}

TEST(LoggerIntegrationTest, MultipleAdaptersAndFlushTiming) {
    // Two adapters sharing the singleton
    LoggerAdapter adapter1;
    LoggerAdapter adapter2;
    adapter1.log(LogLevel::INFO, "from adapter1");
    adapter2.log(LogLevel::ERROR, "from adapter2");
    adapter1.flush();
    adapter2.flush();
    adapter1.shutdown();
    adapter2.shutdown();

    // Verify flush actually waits for processing
    LoggerAdapter timedAdapter;
    for (int i = 0; i < 5; ++i) {
        timedAdapter.log(LogLevel::INFO, "timing_" + std::to_string(i));
    }
    auto start = std::chrono::steady_clock::now();
    timedAdapter.flush();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    EXPECT_GE(elapsed, 50) << "Flush returned too fast — messages may not have been processed";
    timedAdapter.shutdown();
}

// --- Singleton Test ---

TEST(LegacyLoggerTest, SingletonReturnsSameInstance) {
    auto& inst1 = LegacyLogger::getInstance();
    auto& inst2 = LegacyLogger::getInstance();
    EXPECT_EQ(&inst1, &inst2);
}
