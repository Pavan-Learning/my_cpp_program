#include "support/check.hpp"

#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>

std::optional<int> largest(const std::vector<int>& values) {
    if (values.empty()) { return std::nullopt; }
    return *std::max_element(values.begin(), values.end());
}

// Deliberately oversimplified: zero is neither a valid initial maximum for every
// input nor an unambiguous way to say "no values".
int misleading_largest(const std::vector<int>& values) {
    int best = 0;
    for (const int value : values) { if (value > best) { best = value; } }
    return best;
}

void demonstrate_drawback() {
    check(misleading_largest({-8, -2, -5}) == 0, "Short function invents an answer not in the input");
    check(misleading_largest({}) == misleading_largest({0}), "Empty and real zero become indistinguishable");
    check(largest({-8, -2, -5}) == -2 && !largest({}) && largest({0}) == 0,
          "The slightly richer result preserves all three meanings");
    // Drawback of misusing KISS: deleting necessary cases makes callers guess.
    // Simplicity means a clear CORRECT contract, not the fewest lines or checks.
    std::cout << "Drawback: oversimplified maximum returns 0 for negative input "
                 "and hides empty input; optional preserves the difference.\n";
}

int main() {
    check(!largest({}), "Empty input is explicit, not a magic sentinel");
    check(largest({-8, -2, -5}) == -2, "Negative values are valid");
    check(largest({3, 9, 9, 1}) == 9, "Duplicates do not require special machinery");
    std::cout << "Largest: " << *largest({3, 9, 1}) << '\n';
    demonstrate_drawback();
}