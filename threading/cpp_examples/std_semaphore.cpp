#include <array>
#include <atomic>
#include <future>
#include <iostream>
#include <semaphore>
#include <thread>

class Permit
{
    std::counting_semaphore<2>& slots_;

public:
    explicit Permit(std::counting_semaphore<2>& slots) : slots_(slots)
    {
        slots_.acquire();
    }

    ~Permit()
    {
        slots_.release();
    }

    Permit(const Permit&) = delete;
    Permit& operator=(const Permit&) = delete;
};

int main()
{
    std::counting_semaphore<2> slots{2};
    std::atomic<int> active{0};
    std::array<std::future<bool>, 4> tasks;
    for (auto& task : tasks)
    {
        task = std::async(std::launch::async, [&]
        {
            Permit permit(slots);
            const int inside = active.fetch_add(1) + 1;
            const bool within_limit = inside <= 2;
            active.fetch_sub(1);
            return within_limit;
        });
    }
    bool limit_respected = true;
    for (auto& task : tasks)
    {
        const bool valid = task.get();
        limit_respected = limit_respected && valid;
    }

    std::binary_semaphore ready{0};
    int payload = 0;
    std::jthread producer([&]
    {
        payload = 42;
        ready.release();
    });
    ready.acquire();
    const int received = payload;
    producer.join();
    std::cout << "Limit respected: " << std::boolalpha << limit_respected << '\n';
    std::cout << "Completed tasks: " << tasks.size() << '\n';
    std::cout << "Published value: " << received << '\n';
    return limit_respected && active.load() == 0 && received == 42 ? 0 : 1;
}