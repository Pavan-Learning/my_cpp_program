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

namespace before {
// Without a facade, the screen coordinates the services and owns their ordering.
// A second caller would need the same decisions or a shared checkout operation.
std::string checkout_screen(Inventory& inventory, Payment& payment, bool approved) {
    if (!inventory.available()) { return "out of stock"; }
    if (!payment.charge(approved)) { return "payment declined"; }
    inventory.reserve();
    return "order confirmed";
}
}

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

namespace after {
// Clients request the complete job; only CheckoutFacade knows the service sequence.
std::string checkout_screen(CheckoutFacade& checkout, bool approved) {
    return checkout.checkout(approved);
}

std::string kiosk_checkout(CheckoutFacade& checkout, bool approved) {
    return checkout.checkout(approved);
}
}

void demonstrate_before_after() {
    Inventory direct_inventory;
    Payment direct_payment;
    Inventory shared_inventory;
    Payment shared_payment;
    CheckoutFacade checkout(shared_inventory, shared_payment);

    // Independent service objects let us compare the same starting conditions.
    check(before::checkout_screen(direct_inventory, direct_payment, false) ==
              after::checkout_screen(checkout, false),
          "Declined-payment result is preserved");
    check(direct_inventory.remaining() == 1 && shared_inventory.remaining() == 1 &&
              direct_payment.charges() == 0 && shared_payment.charges() == 0,
          "Neither version charges or reserves after a declined payment");

    const auto direct_result = before::checkout_screen(direct_inventory, direct_payment, true);
    const auto facade_result = after::checkout_screen(checkout, true);
    check(direct_result == "order confirmed" && facade_result == direct_result,
          "Facade preserves the successful checkout result");
    check(direct_inventory.remaining() == 0 && shared_inventory.remaining() == 0 &&
              direct_payment.charges() == 1 && shared_payment.charges() == 1,
          "Both versions charge and reserve exactly once");

    // The kiosk reuses the same facade and services: it sees stock already sold.
    check(after::kiosk_checkout(checkout, true) == "out of stock" &&
              before::checkout_screen(direct_inventory, direct_payment, true) == "out of stock",
          "Both versions reject an order after stock is exhausted");
    check(direct_payment.charges() == 1 && shared_payment.charges() == 1,
          "Out-of-stock rejection happens before another charge");
    std::cout << "Without facade: screen coordinates Inventory and Payment -> " << direct_result << '\n';
    std::cout << "With facade: screen and kiosk call checkout() -> " << facade_result << '\n';
}

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
    demonstrate_before_after();
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