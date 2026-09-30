#include <future>
#include <iostream>
#include <stdexcept>
#include <utility>

int main()
{
    std::packaged_task<int()> task([] { return 6 * 7; });
    std::shared_future<int> answer = task.get_future().share();
    auto execution = std::async(std::launch::async, std::move(task));
    auto first_reader = std::async(std::launch::async, [answer] { return answer.get(); });
    auto second_reader = std::async(std::launch::async, [answer] { return answer.get(); });
    const int first = first_reader.get();
    const int second = second_reader.get();
    execution.get();

    std::packaged_task<int()> failing_task([]() -> int
    {
        throw std::runtime_error("Task failed");
    });
    auto failure = failing_task.get_future();
    failing_task();
    bool exception_received = false;
    try
    {
        failure.get();
    }
    catch (const std::runtime_error& error)
    {
        exception_received = true;
        std::cout << "Task error: " << error.what() << '\n';
    }
    std::cout << "Reader results: " << first << ", " << second << '\n';
    std::cout << "Read again: " << answer.get() << '\n';
    return first == 42 && second == 42 && answer.get() == 42 && exception_received ? 0 : 1;
}