#include <array>
#include <atomic>
#include <future>
#include <iostream>
#include <thread>

int main()
{
    std::array<int, 3> payload{};
    std::atomic<bool> ready{false};
    auto producer = std::async(std::launch::async, [&]
    {
        payload = {3, 7, 11};
        ready.store(true, std::memory_order_release);
    });
    while (!ready.load(std::memory_order_acquire))
    {
        std::this_thread::yield();
    }
    const int total = payload[0] + payload[1] + payload[2];
    producer.get();
    std::cout << "Published sum: " << total << '\n';
    return total == 21 ? 0 : 1;
}