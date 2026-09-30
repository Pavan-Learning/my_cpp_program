#include <array>
#include <atomic>
#include <iostream>
#include <thread>

int main()
{
    std::array<int, 3> payload{};
    std::atomic<bool> ready{false};
    std::jthread producer([&]
    {
        payload = {5, 8, 13};
        ready.store(true, std::memory_order_release);
        ready.notify_one();
    });
    ready.wait(false, std::memory_order_acquire);
    const int total = payload[0] + payload[1] + payload[2];
    producer.join();
    std::cout << "Published sum: " << total << '\n';
    return total == 26 ? 0 : 1;
}