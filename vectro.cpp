#include <iostream>
#include <vector>
#include <algorithm>

int main()
{
std::vector<int> vec{4,2,3,5,1};

std::sort(vec.begin(), vec.end());
for(auto i : vec)
{
    std::cout << i << " ";
}

std::cout << std::endl;
for(auto i : vec)
{
    i += 2;
    std::cout << i << " ";
}

std::cout << std::endl;
for(auto i : vec)
{
    i += 2;
}

for(auto i : vec)
{
    std::cout << i << " ";
}

for (auto& i : vec)
{
    i += 2;
}

for(auto i : vec)
{
    std::cout << i << " ";
}
return 0;
}