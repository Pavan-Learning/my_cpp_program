#include <atomic>
#include <future>
#include <iostream>

bool withdraw(std::atomic<int>& balance, int amount)
{
    int observed = balance.load(std::memory_order_relaxed);
    while (observed >= amount)
    {
        if (balance.compare_exchange_weak(observed, observed - amount,
                                         std::memory_order_relaxed))
        {
            return true;
        }
    }
    return false;
}

int main()
{
    std::atomic<int> counter{0};
    std::atomic<int> balance{100};
    auto work = [&]
    {
        for (int iteration = 0; iteration < 10000; ++iteration)
        {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
        return withdraw(balance, 60);
    };
    auto first = std::async(std::launch::async, work);
    auto second = std::async(std::launch::async, work);
    const bool first_paid = first.get();
    const bool second_paid = second.get();
    const int successes = static_cast<int>(first_paid) + static_cast<int>(second_paid);
    const int total = counter.load(std::memory_order_relaxed);
    const int remaining = balance.load(std::memory_order_relaxed);
    std::cout << "Counter: " << total << '\n';
    std::cout << "Successful withdrawals: " << successes << '\n';
    std::cout << "Remaining balance: " << remaining << '\n';
    return total == 20000 && successes == 1 && remaining == 40 ? 0 : 1;
}