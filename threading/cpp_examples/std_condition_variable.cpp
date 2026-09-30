#include <condition_variable>
#include <deque>
#include <future>
#include <iostream>
#include <mutex>
#include <optional>

class BoundedQueue
{
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    std::deque<int> items_;
    bool closed_ = false;

public:
    bool push(int value)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [&] { return closed_ || items_.size() < 3; });
        if (closed_)
        {
            return false;
        }
        items_.push_back(value);
        lock.unlock();
        not_empty_.notify_one();
        return true;
    }

    std::optional<int> pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [&] { return closed_ || !items_.empty(); });
        if (items_.empty())
        {
            return std::nullopt;
        }
        const int value = items_.front();
        items_.pop_front();
        lock.unlock();
        not_full_.notify_one();
        return value;
    }

    void close()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        not_empty_.notify_all();
        not_full_.notify_all();
    }
};

int main()
{
    BoundedQueue queue;
    auto consumer = std::async(std::launch::async, [&]
    {
        int total = 0;
        while (auto value = queue.pop())
        {
            total += *value;
        }
        return total;
    });
    try
    {
        for (int value = 1; value <= 10; ++value)
        {
            queue.push(value);
        }
    }
    catch (...)
    {
        queue.close();
        throw;
    }
    queue.close();
    const int total = consumer.get();
    const bool rejected = !queue.push(11);
    const bool drained = !queue.pop().has_value();
    std::cout << "Consumed sum: " << total << '\n';
    std::cout << "Closed and drained: " << std::boolalpha << (rejected && drained) << '\n';
    return total == 55 && rejected && drained ? 0 : 1;
}