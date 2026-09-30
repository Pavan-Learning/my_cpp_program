#include <future>
#include <iostream>

thread_local int processed = 0;

int process_batch(int count)
{
    for (int item = 0; item < count; ++item)
    {
        ++processed;
    }
    return processed;
}

int main()
{
    process_batch(1);
    auto first = std::async(std::launch::async, [] { return process_batch(3); });
    auto second = std::async(std::launch::async, [] { return process_batch(5); });
    const int first_count = first.get();
    const int second_count = second.get();
    std::cout << "Main thread count: " << processed << '\n';
    std::cout << "Worker counts: " << first_count << ", " << second_count << '\n';
    std::cout << "Combined count: " << processed + first_count + second_count << '\n';
    return processed == 1 && first_count == 3 && second_count == 5 ? 0 : 1;
}