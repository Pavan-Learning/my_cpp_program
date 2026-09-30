#include "support/check.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

std::string stock_report(int available) {
    return "Available: " + std::to_string(available);
}

std::string report_valid_stock(int available) {
    if (available < 0) { throw std::invalid_argument("Stock cannot be negative"); }
    return stock_report(available);
}

void demonstrate_drawback() {
    // Suppose the input boundary must reject negative stock. That is a KNOWN
    // requirement, not a speculative future feature. The formatter alone cannot
    // enforce it; calling it directly would happily display the invalid value.
    check(stock_report(-1) == "Available: -1", "Formatting alone is not stock validation");
    bool rejected = false;
    try { static_cast<void>(report_valid_stock(-1)); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected && report_valid_stock(12) == "Available: 12", "Required validation needs explicit implementation");

    // Drawback of misusing YAGNI: postponing required safety or compatibility is
    // not removing speculation. Add the small known check, without inventing a
    // plugin framework, database, or other features this report does not need.
    std::cout << "Drawback: treating required validation as 'not needed yet' "
                 "allows negative stock; the checked boundary rejects it.\n";
}

int main() {
    check(stock_report(0) == "Available: 0", "Current empty-stock requirement");
    check(stock_report(12) == "Available: 12", "Current text-report requirement");
    std::cout << stock_report(12) << '\n';
    demonstrate_drawback();
}