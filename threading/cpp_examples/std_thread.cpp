#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>

int run_worker(int input)
{
    int result = 0;
    std::exception_ptr failure;
    std::thread worker([&failure](int value, int& output)
    {
        try
        {
            if (value < 0)
            {
                throw std::invalid_argument("Input must be nonnegative");
            }
            output = value * value;
        }
        catch (...)
        {
            failure = std::current_exception();
        }
    }, input, std::ref(result));

    std::thread owner = std::move(worker);
    owner.join();
    if (failure)
    {
        std::rethrow_exception(failure);
    }
    return result;
}

int main()
{
    const int result = run_worker(6);
    bool error_received = false;
    try
    {
        run_worker(-1);
    }
    catch (const std::invalid_argument& error)
    {
        error_received = true;
        std::cout << "Worker error: " << error.what() << '\n';
    }
    std::cout << "Square: " << result << '\n';
    return result == 36 && error_received ? 0 : 1;
}