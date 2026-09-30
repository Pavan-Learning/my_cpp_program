#include "support/check.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace before {
enum class Customer { regular, member };
int price(int cents, Customer customer) {
    switch (customer) {
        case Customer::regular: return cents;
        case Customer::member: return cents - cents / 10;
    }
    throw std::invalid_argument("Unknown customer");
}
}

namespace after {
struct PricingRule {
    virtual ~PricingRule() = default;
    virtual int apply(int cents) const = 0;
};

struct Regular final : PricingRule {
    int apply(int cents) const override { return cents; }
};

struct Member final : PricingRule {
    int apply(int cents) const override { return cents - cents / 10; }
};

struct Festival final : PricingRule {
    int apply(int cents) const override { return cents - cents / 5; }
};

int price(int cents, const PricingRule& rule) {
    if (cents < 0) { throw std::invalid_argument("Negative price"); }
    return rule.apply(cents);
}
}

void demonstrate_drawback() {
    const after::Member member;
    const after::Festival festival;
    const int combined = after::price(after::price(1000, member), festival);
    const int best_single = std::min(after::price(1000, member), after::price(1000, festival));
    check(combined == 720 && best_single == 800, "Combining rules differs from choosing one rule");

    // Drawback: this extension point was designed for ONE pricing rule. Adding
    // implementations does not answer a new question: should discounts stack or
    // should only the best apply? Both calls compile, yet give different answers.
    // Setup/composition needs a business decision; OCP does not mean never edit
    // code or invent interfaces for every possible future requirement.
    std::cout << "Drawback: adding rules does not choose how to combine them; "
                 "stacked discounts=720, best single discount=800 cents.\n";
}

int main() {
    check(after::price(1000, after::Regular{}) == before::price(1000, before::Customer::regular), "Regular behavior preserved");
    check(after::price(1000, after::Member{}) == before::price(1000, before::Customer::member), "Member behavior preserved");
    check(after::price(1000, after::Festival{}) == 800, "New rule without editing price algorithm");
    std::cout << "Festival price: " << after::price(1000, after::Festival{}) << '\n';
    demonstrate_drawback();
}