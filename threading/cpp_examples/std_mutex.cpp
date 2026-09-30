#include <future>
#include <iostream>
#include <mutex>

int main()
{
    std::mutex counter_mutex;
    int counter = 0;
    auto increment = [&]
    {
        for (int iteration = 0; iteration < 10000; ++iteration)
        {
            std::lock_guard<std::mutex> guard(counter_mutex);
            ++counter;
        }
    };
    auto first = std::async(std::launch::async, increment);
    auto second = std::async(std::launch::async, increment);
    first.get();
    second.get();

    std::unique_lock<std::mutex> snapshot_lock(counter_mutex);
    const int snapshot = counter;
    snapshot_lock.unlock();

    std::mutex first_mutex;
    std::mutex second_mutex;
    int first_balance = 500;
    int second_balance = 600;
    {
        std::scoped_lock guards(first_mutex, second_mutex);
        first_balance -= 50;
        second_balance += 50;
    }
    std::cout << "Counter: " << snapshot << '\n';
    std::cout << "Balances: " << first_balance << ", " << second_balance << '\n';
    return snapshot == 20000 && first_balance == 450 && second_balance == 650 ? 0 : 1;
}