#include "support/check.hpp"

#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

int parse_weight(std::string_view text) {
    if (text.empty()) { throw std::invalid_argument("Empty weight"); }
    int weight = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), weight);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
        throw std::invalid_argument("Expected an integer");
    }
    return weight;
}

int shipping_cents(int weight) {
    if (weight <= 0 || weight > 1000) { throw std::invalid_argument("Weight out of range"); }
    return 300 + weight * 50;
}

std::string display_price(int cents) { return std::to_string(cents) + " cents"; }

int main() {
    check(parse_weight("2") == 2, "Parsing boundary");
    check(shipping_cents(2) == 400, "Business rule without input or output");
    check(display_price(400) == "400 cents", "Presentation boundary");
    bool rejected = false;
    try { parse_weight("2kg"); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Do not silently accept partial input");
    std::cout << display_price(shipping_cents(parse_weight("2"))) << '\n';
}