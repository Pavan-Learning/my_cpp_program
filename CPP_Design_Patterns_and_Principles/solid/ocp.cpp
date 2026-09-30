#include "support/check.hpp"

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

int main() {
    check(after::price(1000, after::Regular{}) == before::price(1000, before::Customer::regular), "Regular behavior preserved");
    check(after::price(1000, after::Member{}) == before::price(1000, before::Customer::member), "Member behavior preserved");
    check(after::price(1000, after::Festival{}) == 800, "New rule without editing price algorithm");
    std::cout << "Festival price: " << after::price(1000, after::Festival{}) << '\n';
}