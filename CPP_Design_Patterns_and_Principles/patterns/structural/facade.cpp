#include "support/check.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

class Inventory {
public:
    bool available() const { return stock_ > 0; }
    void reserve() {
        if (!available()) { throw std::logic_error("Out of stock"); }
        --stock_;
    }
    int remaining() const { return stock_; }

private:
    int stock_ = 1;
};

class Payment {
public:
    bool charge(bool approved) { return approved; }
};

class CheckoutFacade {
public:
    CheckoutFacade(Inventory& inventory, Payment& payment) : inventory_(inventory), payment_(payment) {}
    std::string checkout(bool payment_approved) {
        if (!inventory_.available()) { return "out of stock"; }
        if (!payment_.charge(payment_approved)) { return "payment declined"; }
        inventory_.reserve();
        return "order confirmed";
    }

private:
    Inventory& inventory_;
    Payment& payment_;
};

int main() {
    Inventory inventory;
    Payment payment;
    CheckoutFacade checkout(inventory, payment);
    check(checkout.checkout(false) == "payment declined", "Failed payment is reported");
    check(inventory.remaining() == 1, "Failed payment does not consume stock");
    check(checkout.checkout(true) == "order confirmed", "Successful orchestration");
    check(checkout.checkout(true) == "out of stock", "Stock cannot be sold twice");
    std::cout << "Checkout success and failure paths verified\n";
}