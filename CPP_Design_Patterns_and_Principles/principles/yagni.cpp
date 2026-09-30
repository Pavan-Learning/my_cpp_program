#include "support/check.hpp"

#include <iostream>
#include <string>

std::string stock_report(int available) {
    return "Available: " + std::to_string(available);
}

int main() {
    check(stock_report(0) == "Available: 0", "Current empty-stock requirement");
    check(stock_report(12) == "Available: 12", "Current text-report requirement");
    std::cout << stock_report(12) << '\n';
}