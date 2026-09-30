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

void demonstrate_drawback() {
    // Suppose free shipping and loyalty rewards originally both started at 5000.
    // They LOOK alike but belong to different rules. Only shipping moves to 6000.
    const int incorrectly_shared_threshold = 6000;
    const auto merged_eligibility = [incorrectly_shared_threshold](int subtotal) {
        return subtotal >= incorrectly_shared_threshold;
    };
    const bool shipping = merged_eligibility(5500);
    const bool loyalty = merged_eligibility(5500);
    const bool independent_loyalty = 5500 >= 5000;
    check(!shipping && !loyalty && independent_loyalty, "False sharing accidentally changes loyalty eligibility");

    // Drawback of over-applying DRY: changing ONE business rule changes another
    // unrelated rule. Keep separate meanings even when today's arithmetic matches.
    // Web and kiosk shipping above really do share the SAME shipping rule.
    std::cout << "Drawback: a shared 6000 threshold wrongly denies loyalty at 5500; "
                 "loyalty's independent 5000 threshold should still qualify.\n";
}

int main() {
    const ShippingRules rules;
    check(web_total(4999, rules) == 5499, "Just below threshold");
    check(kiosk_total(5000, rules) == 5000, "At threshold");
    check(web_total(2000, rules) == kiosk_total(2000, rules), "Both clients use one business rule");
    std::cout << "Shared shipping rule verified\n";
    demonstrate_drawback();
}