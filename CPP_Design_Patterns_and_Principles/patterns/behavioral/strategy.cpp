#include "support/check.hpp"

#include <iostream>
#include <stdexcept>

struct ShippingPolicy {
    virtual ~ShippingPolicy() = default;
    virtual int cents(int kilograms) const = 0;
};

struct StandardShipping final : ShippingPolicy {
    int cents(int kilograms) const override { return 300 + kilograms * 50; }
};

struct ExpressShipping final : ShippingPolicy {
    int cents(int kilograms) const override { return 600 + kilograms * 100; }
};

class Checkout {
public:
    explicit Checkout(const ShippingPolicy& policy) : policy_(&policy) {}
    void use(const ShippingPolicy& policy) { policy_ = &policy; }
    int shipping(int kilograms) const {
        if (kilograms <= 0 || kilograms > 1000) { throw std::invalid_argument("Weight out of range"); }
        return policy_->cents(kilograms);
    }

private:
    const ShippingPolicy* policy_;
};

void demonstrate_drawback() {
    const StandardShipping standard;
    const ExpressShipping express;
    Checkout checkout(standard);

    // Drawback: the customer asked for express, but setup forgot to select it.
    // The weight is valid and both policies satisfy the interface, so Checkout
    // cannot discover this business mistake by checking the number alone.
    const int wrong_quote = checkout.shipping(2);
    check(wrong_quote == 400 && wrong_quote != express.cents(2), "Valid policy can be the wrong selected policy");
    checkout.use(express);
    check(checkout.shipping(2) == 800, "Explicit selection applies the requested policy");
    std::cout << "Drawback: forgotten policy selection quotes 400 instead of "
                 "the requested express price of 800 cents.\n";
    // These borrowed policies stay alive until checkout is finished. Strategy
    // does not extend their lifetime; passing a destroyed policy would be unsafe.
}

int main() {
    const StandardShipping standard;
    const ExpressShipping express;
    Checkout checkout(standard);
    check(checkout.shipping(2) == 400, "Standard policy");
    checkout.use(express);
    check(checkout.shipping(2) == 800, "Policy can change without changing Checkout");
    bool rejected = false;
    try { checkout.shipping(0); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Context enforces the common input contract");
    std::cout << "Express: " << checkout.shipping(2) << " cents\n";
    demonstrate_drawback();
}