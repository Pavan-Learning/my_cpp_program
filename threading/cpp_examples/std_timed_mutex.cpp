#include <chrono>
#include <future>
#include <iostream>
#include <mutex>

int main()
{
    std::timed_mutex resource_mutex;
    bool unavailable = false;
    {
        std::unique_lock<std::timed_mutex> owner(resource_mutex);
        auto contender = std::async(std::launch::async, [&]
        {
            std::unique_lock<std::timed_mutex> attempt(resource_mutex, std::defer_lock);
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(10);
            return !attempt.try_lock_until(deadline);
        });
        unavailable = contender.get();
    }
    std::unique_lock<std::timed_mutex> next_owner(resource_mutex);
    const bool acquired = next_owner.owns_lock();
    next_owner.unlock();
    std::cout << "Contended attempt failed: " << std::boolalpha << unavailable << '\n';
    std::cout << "Acquired after release: " << acquired << '\n';
    return unavailable && acquired ? 0 : 1;
}