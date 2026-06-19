#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, int> mymap =
{
    {"pavan", 1},
    {"sai", 2},
    {"kumar", 3}
};

int main()
{
    for (const std::pair<std::string, int>& pair : mymap)
    {
        std::cout << &pair  << std::endl;
    }

    std::cout <<"***************************************" <<std::endl;
    for (const auto& pair : mymap)
    {
        std::cout << &pair  << std::endl;
    }

    return 0;
}