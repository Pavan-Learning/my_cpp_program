#include <array>
#include <barrier>
#include <iostream>
#include <latch>
#include <thread>

int main()
{
    std::array<int, 3> partials{};
    int grand_total = 0;
    int completed_phases = 0;
    std::barrier phase_done(3, [&]() noexcept
    {
        for (int value : partials)
        {
            grand_total += value;
        }
        ++completed_phases;
    });
    std::latch start{1};
    bool all_started = false;
    std::array<std::jthread, 3> workers;
    try
    {
        for (std::size_t index = 0; index < workers.size(); ++index)
        {
            workers[index] = std::jthread([&, index]
            {
                start.wait();
                if (!all_started)
                {
                    return;
                }
                for (int phase = 1; phase <= 2; ++phase)
                {
                    partials[index] = (static_cast<int>(index) + 1) * phase;
                    phase_done.arrive_and_wait();
                }
            });
        }
    }
    catch (...)
    {
        start.count_down();
        throw;
    }
    all_started = true;
    start.count_down();
    for (auto& worker : workers)
    {
        worker.join();
    }
    std::cout << "Completed phases: " << completed_phases << '\n';
    std::cout << "Combined phase totals: " << grand_total << '\n';
    return completed_phases == 2 && grand_total == 18 ? 0 : 1;
}