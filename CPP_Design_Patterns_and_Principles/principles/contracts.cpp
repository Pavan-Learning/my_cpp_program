#include "support/check.hpp"

#include <iostream>
#include <stdexcept>

class Percentage {
public:
    explicit Percentage(int value) : value_(value) {
        if (value < 0 || value > 100) { throw std::invalid_argument("Percentage must be 0..100"); }
    }
    int of(int cents) const {
        if (cents < 0 || cents > 1000000) { throw std::invalid_argument("Amount outside demo contract"); }
        return static_cast<int>(static_cast<long long>(cents) * value_ / 100);
    }

private:
    int value_;
};

int main() {
    check(Percentage(0).of(1000) == 0, "Lower boundary");
    check(Percentage(100).of(1000) == 1000, "Upper boundary");
    check(Percentage(25).of(999) == 249, "Documented integer truncation");
    bool rejected = false;
    try { const Percentage invalid(101); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Invalid value cannot become a usable Percentage");
    std::cout << "25 percent of 999 cents: " << Percentage(25).of(999) << '\n';
}