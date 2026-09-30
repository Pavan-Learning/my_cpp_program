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

void demonstrate_drawback() {
    int validations = 0;
    const auto validate_percentage = [&validations](int value) {
        ++validations;
        return Percentage(value);
    };
    int repeated_total = 0;
    for (int item = 0; item < 3; ++item) { repeated_total += validate_percentage(25).of(100); }
    check(validations == 3 && repeated_total == 75, "Reconstructing the same percentage repeats its validation");

    validations = 0;
    const auto rate = validate_percentage(25);
    int reused_total = 0;
    for (int item = 0; item < 3; ++item) { reused_total += rate.of(100); }
    check(validations == 1 && reused_total == repeated_total, "Reusing a validated value preserves the result");

    // Drawback: needless revalidation adds work and noise. Validate the percentage
    // at its input boundary, then retain its meaningful type. Each DIFFERENT amount
    // still needs of()'s range check; trusting one value does not validate all inputs.
    // We count percentage validations, not elapsed time or every check in of().
    std::cout << "Drawback: rebuilding the same rate validates it three times; "
                 "reusing Percentage validates it once for the same total of 75.\n";
}

int main() {
    check(Percentage(0).of(1000) == 0, "Lower boundary");
    check(Percentage(100).of(1000) == 1000, "Upper boundary");
    check(Percentage(25).of(999) == 249, "Documented integer truncation");
    bool rejected = false;
    try { const Percentage invalid(101); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Invalid value cannot become a usable Percentage");
    std::cout << "25 percent of 999 cents: " << Percentage(25).of(999) << '\n';
    demonstrate_drawback();
}