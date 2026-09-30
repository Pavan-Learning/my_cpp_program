#include "support/check.hpp"

#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>

std::optional<int> largest(const std::vector<int>& values) {
    if (values.empty()) { return std::nullopt; }
    return *std::max_element(values.begin(), values.end());
}

int main() {
    check(!largest({}), "Empty input is explicit, not a magic sentinel");
    check(largest({-8, -2, -5}) == -2, "Negative values are valid");
    check(largest({3, 9, 9, 1}) == 9, "Duplicates do not require special machinery");
    std::cout << "Largest: " << *largest({3, 9, 1}) << '\n';
}