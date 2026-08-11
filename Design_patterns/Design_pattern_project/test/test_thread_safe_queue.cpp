#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <set>
#include <atomic>
#include "../inc/Thread_safe_queue.hpp"

class ThreadSafeQueueTest : public ::testing::Test {
protected:
    ThreadSafeQueue queue;
};

// --- Basic Operations ---

TEST_F(ThreadSafeQueueTest, StartsEmpty) {
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_EQ(queue.getSize(), 0u);
}

TEST_F(ThreadSafeQueueTest, PushIncrementsSize) {
    queue.push("msg1", LogLevel::INFO);
    EXPECT_EQ(queue.getSize(), 1u);
    EXPECT_FALSE(queue.isEmpty());

    queue.push("msg2", LogLevel::ERROR);
    EXPECT_EQ(queue.getSize(), 2u);
}

TEST_F(ThreadSafeQueueTest, PopReturnsFIFOOrder) {
    queue.push("first", LogLevel::DEBUG);
    queue.push("second", LogLevel::WARNING);
    queue.push("third", LogLevel::ERROR);

    auto entry1 = queue.pop();
    ASSERT_TRUE(entry1.has_value());
    EXPECT_EQ(entry1->first, "first");
    EXPECT_EQ(entry1->second, LogLevel::DEBUG);

    auto entry2 = queue.pop();
    ASSERT_TRUE(entry2.has_value());
    EXPECT_EQ(entry2->first, "second");
    EXPECT_EQ(entry2->second, LogLevel::WARNING);

    auto entry3 = queue.pop();
    ASSERT_TRUE(entry3.has_value());
    EXPECT_EQ(entry3->first, "third");
    EXPECT_EQ(entry3->second, LogLevel::ERROR);
}

TEST_F(ThreadSafeQueueTest, PopDecrementsSizeToEmpty) {
    queue.push("msg", LogLevel::INFO);
    queue.pop();
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_EQ(queue.getSize(), 0u);
}

// --- All Log Levels ---

TEST_F(ThreadSafeQueueTest, AllLogLevelsStoredCorrectly) {
    queue.push("d", LogLevel::DEBUG);
    queue.push("i", LogLevel::INFO);
    queue.push("w", LogLevel::WARNING);
    queue.push("e", LogLevel::ERROR);
    queue.push("c", LogLevel::CRITICAL);

    EXPECT_EQ(queue.pop()->second, LogLevel::DEBUG);
    EXPECT_EQ(queue.pop()->second, LogLevel::INFO);
    EXPECT_EQ(queue.pop()->second, LogLevel::WARNING);
    EXPECT_EQ(queue.pop()->second, LogLevel::ERROR);
    EXPECT_EQ(queue.pop()->second, LogLevel::CRITICAL);
}

// --- Shutdown Behavior ---

TEST_F(ThreadSafeQueueTest, PopReturnsNulloptAfterShutdownOnEmptyQueue) {
    queue.setShutdown();
    auto result = queue.pop();
    EXPECT_FALSE(result.has_value());
}

TEST_F(ThreadSafeQueueTest, PopDrainsRemainingBeforeShutdown) {
    queue.push("msg1", LogLevel::INFO);
    queue.push("msg2", LogLevel::ERROR);
    queue.setShutdown();

    // Existing entries should still be poppable
    auto r1 = queue.pop();
    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(r1->first, "msg1");

    auto r2 = queue.pop();
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r2->first, "msg2");

    // Now empty + shutdown -> nullopt
    auto r3 = queue.pop();
    EXPECT_FALSE(r3.has_value());
}

TEST_F(ThreadSafeQueueTest, PushIgnoredAfterShutdown) {
    queue.setShutdown();
    queue.push("should_be_dropped", LogLevel::INFO);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(ThreadSafeQueueTest, ShutdownUnblocksBlockedPop) {
    // pop() on empty queue blocks; shutdown should unblock it
    std::atomic<bool> popped{false};
    std::optional<std::pair<std::string, LogLevel>> result;

    std::thread consumer([&]() {
        result = queue.pop();
        popped = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(popped.load());

    queue.setShutdown();
    consumer.join();

    EXPECT_TRUE(popped.load());
    EXPECT_FALSE(result.has_value());
}

// --- Concurrent Push/Pop ---

TEST_F(ThreadSafeQueueTest, ConcurrentPushPopNoDataLoss) {
    constexpr int numProducers = 4;
    constexpr int msgsPerProducer = 50;
    std::atomic<int> consumed{0};
    std::vector<std::thread> producers;

    std::thread consumer([&]() {
        while (true) {
            auto entry = queue.pop();
            if (!entry.has_value()) break;
            consumed++;
        }
    });

    for (int i = 0; i < numProducers; ++i) {
        producers.emplace_back([&, i]() {
            for (int j = 0; j < msgsPerProducer; ++j) {
                queue.push("p" + std::to_string(i) + "_" + std::to_string(j), LogLevel::INFO);
            }
        });
    }

    for (auto& t : producers) t.join();
    queue.setShutdown();
    consumer.join();

    EXPECT_EQ(consumed.load(), numProducers * msgsPerProducer);
}

TEST_F(ThreadSafeQueueTest, MultipleProducersSingleConsumerOrdering) {
    // Each producer pushes sequential numbers; consumer should see each producer's messages in order
    constexpr int numProducers = 3;
    constexpr int msgsPerProducer = 20;
    std::vector<std::vector<int>> perProducerOrder(numProducers);
    std::mutex orderMtx;
    std::vector<std::thread> producers;

    std::thread consumer([&]() {
        while (true) {
            auto entry = queue.pop();
            if (!entry.has_value()) break;
            // Message format: "P<id>_<seq>"
            auto& msg = entry->first;
            size_t pPos = msg.find('P');
            size_t uPos = msg.find('_');
            int pid = std::stoi(msg.substr(pPos + 1, uPos - pPos - 1));
            int seq = std::stoi(msg.substr(uPos + 1));
            std::lock_guard<std::mutex> lk(orderMtx);
            perProducerOrder[pid].push_back(seq);
        }
    });

    for (int i = 0; i < numProducers; ++i) {
        producers.emplace_back([&, i]() {
            for (int j = 0; j < msgsPerProducer; ++j) {
                queue.push("P" + std::to_string(i) + "_" + std::to_string(j), LogLevel::DEBUG);
            }
        });
    }

    for (auto& t : producers) t.join();
    queue.setShutdown();
    consumer.join();

    for (int i = 0; i < numProducers; ++i) {
        ASSERT_EQ(static_cast<int>(perProducerOrder[i].size()), msgsPerProducer)
            << "Producer " << i << " lost messages";
        for (int j = 0; j < msgsPerProducer; ++j) {
            EXPECT_EQ(perProducerOrder[i][j], j)
                << "Producer " << i << " messages out of order";
        }
    }
}

// --- Back-Pressure (bounded queue) ---

TEST_F(ThreadSafeQueueTest, BackPressureBlocksProducerWhenFull) {
    // Fill queue to capacity
    for (size_t i = 0; i < 100; ++i) {
        queue.push("fill_" + std::to_string(i), LogLevel::INFO);
    }
    EXPECT_EQ(queue.getSize(), 100u);

    std::atomic<bool> pushDone{false};
    std::thread producer([&]() {
        queue.push("overflow", LogLevel::INFO);
        pushDone = true;
    });

    // Producer should be blocked
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(pushDone.load());

    // Pop one to make space
    queue.pop();
    producer.join();

    EXPECT_TRUE(pushDone.load());
    EXPECT_EQ(queue.getSize(), 100u);
}

TEST_F(ThreadSafeQueueTest, ShutdownUnblocksBlockedProducer) {
    for (size_t i = 0; i < 100; ++i) {
        queue.push("fill_" + std::to_string(i), LogLevel::INFO);
    }

    std::atomic<bool> pushReturned{false};
    std::thread producer([&]() {
        queue.push("blocked_msg", LogLevel::INFO);
        pushReturned = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(pushReturned.load());

    queue.setShutdown();
    producer.join();
    EXPECT_TRUE(pushReturned.load());
    // Message should NOT have been added since shutdown was signaled
    EXPECT_EQ(queue.getSize(), 100u);
}

// --- Edge Cases ---

TEST_F(ThreadSafeQueueTest, EmptyStringMessage) {
    queue.push("", LogLevel::INFO);
    auto result = queue.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->first, "");
}

TEST_F(ThreadSafeQueueTest, LargeMessage) {
    std::string largeMsg(10000, 'X');
    queue.push(largeMsg, LogLevel::ERROR);
    auto result = queue.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->first, largeMsg);
}

TEST_F(ThreadSafeQueueTest, DoubleShutdownSafe) {
    queue.setShutdown();
    queue.setShutdown(); // Should not crash
    auto result = queue.pop();
    EXPECT_FALSE(result.has_value());
}
