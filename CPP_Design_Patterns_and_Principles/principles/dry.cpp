#include "support/check.hpp"

#include <iostream>
#include <stdexcept>

class ShippingRules {
public:
    int fee(int subtotal_cents) const {
        if (subtotal_cents < 0) { throw std::invalid_argument("Negative subtotal"); }
        return subtotal_cents >= free_shipping_threshold ? 0 : 500;
    }

private:
    static constexpr int free_shipping_threshold = 5000;
};

int web_total(int subtotal, const ShippingRules& rules) { return subtotal + rules.fee(subtotal); }
int kiosk_total(int subtotal, const ShippingRules& rules) { return subtotal + rules.fee(subtotal); }

int main() {
    const ShippingRules rules;
    check(web_total(4999, rules) == 5499, "Just below threshold");
    check(kiosk_total(5000, rules) == 5000, "At threshold");
    check(web_total(2000, rules) == kiosk_total(2000, rules), "Both clients use one business rule");
    std::cout << "Shared shipping rule verified\n";
}