#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

class ThreadPool
{
    std::mutex mutex_;
    std::condition_variable available_;
    std::deque<std::packaged_task<int()>> tasks_;
    bool closed_ = false;
    std::vector<std::jthread> workers_;

    void run()
    {
        while (true)
        {
            std::packaged_task<int()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                available_.wait(lock, [&] { return closed_ || !tasks_.empty(); });
                if (tasks_.empty())
                {
                    return;
                }
                task = std::move(tasks_.front());
                tasks_.pop_front();
            }
            task();
        }
    }

public:
    explicit ThreadPool(std::size_t worker_count)
    {
        if (worker_count == 0)
        {
            throw std::invalid_argument("At least one worker is required");
        }
        workers_.reserve(worker_count);
        try
        {
            for (std::size_t index = 0; index < worker_count; ++index)
            {
                workers_.emplace_back([this] { run(); });
            }
        }
        catch (...)
        {
            close();
            throw;
        }
    }

    ~ThreadPool()
    {
        close();
        workers_.clear();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    std::future<int> submit(std::function<int()> function)
    {
        std::packaged_task<int()> task(std::move(function));
        auto result = task.get_future();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (closed_)
            {
                throw std::runtime_error("Pool is closed");
            }
            tasks_.push_back(std::move(task));
        }
        available_.notify_one();
        return result;
    }

    void close()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        available_.notify_all();
    }
};

int main()
{
    bool zero_rejected = false;
    try
    {
        ThreadPool invalid(0);
    }
    catch (const std::invalid_argument&)
    {
        zero_rejected = true;
    }

    int sum = 0;
    bool exception_received = false;
    bool late_rejected = false;
    std::future<int> final_result;
    {
        ThreadPool pool(2);
        std::vector<std::future<int>> results;
        for (int value = 1; value <= 4; ++value)
        {
            results.push_back(pool.submit([value] { return value * value; }));
        }
        auto failure = pool.submit([]() -> int { throw std::runtime_error("Task failed"); });
        final_result = pool.submit([] { return 99; });
        pool.close();
        try
        {
            pool.submit([] { return 0; });
        }
        catch (const std::runtime_error&)
        {
            late_rejected = true;
        }
        for (auto& result : results)
        {
            sum += result.get();
        }
        try
        {
            failure.get();
        }
        catch (const std::runtime_error&)
        {
            exception_received = true;
        }
    }
    const int drained_value = final_result.get();
    std::cout << "Square sum: " << sum << '\n';
    std::cout << "Task exception delivered: " << std::boolalpha << exception_received << '\n';
    std::cout << "Late submission rejected: " << late_rejected << '\n';
    std::cout << "Result after pool destruction: " << drained_value << '\n';
    return zero_rejected && sum == 30 && exception_received && late_rejected
        && drained_value == 99 ? 0 : 1;
}