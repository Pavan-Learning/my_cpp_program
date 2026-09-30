#include "support/check.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

class Inventory {
public:
    explicit Inventory(bool fail_reservation = false) : fail_reservation_(fail_reservation) {}
    bool available() const { return stock_ > 0; }
    void reserve() {
        if (!available()) { throw std::logic_error("Out of stock"); }
        // Simulated service failure, not a real stock system or network request.
        if (fail_reservation_) { throw std::runtime_error("Reservation service failed"); }
        --stock_;
    }
    int remaining() const { return stock_; }

private:
    int stock_ = 1;
    bool fail_reservation_;
};

class Payment {
public:
    bool charge(bool approved) {
        if (approved) { ++charges_; }
        return approved;
    }
    int charges() const { return charges_; }

private:
    int charges_ = 0;
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

void demonstrate_drawback() {
    Inventory failing_inventory(true);
    Payment payment;
    CheckoutFacade checkout(failing_inventory, payment);
    bool failed = false;
    try { static_cast<void>(checkout.checkout(true)); }
    catch (const std::runtime_error&) { failed = true; }
    check(failed && payment.charges() == 1 && failing_inventory.remaining() == 1,
          "Reservation failed AFTER the simulated charge succeeded");

    // Drawback: one convenient function is NOT an all-or-nothing transaction.
    // C++ destroys local objects, but that does not refund an external payment.
    // A real checkout needs a recovery/refund policy and protection from duplicate
    // charges on retries. We leave the partial result visible instead of hiding it.
    std::cout << "Drawback: checkout failed, but charges=1 and stock=1; "
                 "the facade did not undo the earlier payment.\n";
}

int main() {
    Inventory inventory;
    Payment payment;
    CheckoutFacade checkout(inventory, payment);
    check(checkout.checkout(false) == "payment declined", "Failed payment is reported");
    check(inventory.remaining() == 1, "Failed payment does not consume stock");
    check(checkout.checkout(true) == "order confirmed", "Successful orchestration");
    check(checkout.checkout(true) == "out of stock", "Stock cannot be sold twice");
    std::cout << "Checkout success and failure paths verified\n";
    demonstrate_drawback();
}