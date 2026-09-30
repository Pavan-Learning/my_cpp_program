#include <future>
#include <iostream>
#include <mutex>
#include <string>

int main()
{
    std::once_flag initialized;
    std::string configuration;
    int attempts = 0;
    auto read_configuration = [&]
    {
        std::call_once(initialized, [&]
        {
            ++attempts;
            configuration = "database=local";
        });
        return configuration;
    };

    auto first = std::async(std::launch::async, read_configuration);
    auto second = std::async(std::launch::async, read_configuration);
    const std::string first_value = first.get();
    const std::string second_value = second.get();
    std::cout << "Initialization attempts: " << attempts << '\n';
    std::cout << "Configuration: " << first_value << '\n';
    return attempts == 1 && first_value == "database=local"
        && second_value == first_value ? 0 : 1;
}