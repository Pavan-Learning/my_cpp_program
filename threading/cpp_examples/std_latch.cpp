#include <array>
#include <future>
#include <iostream>
#include <latch>

int main()
{
    std::array<int, 3> results{};
    std::latch finished{3};
    std::array<std::future<void>, 3> workers;
    for (std::size_t index = 0; index < workers.size(); ++index)
    {
        workers[index] = std::async(std::launch::async, [&, index]
        {
            const int value = static_cast<int>(index) + 1;
            results[index] = value * value;
            finished.count_down();
        });
    }
    finished.wait();
    const int total = results[0] + results[1] + results[2];
    for (auto& worker : workers)
    {
        worker.get();
    }
    std::cout << "Sum of squares: " << total << '\n';
    std::cout << "Latch open: " << std::boolalpha << finished.try_wait() << '\n';
    return total == 14 && finished.try_wait() ? 0 : 1;
}