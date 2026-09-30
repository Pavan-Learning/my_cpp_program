#include "support/check.hpp"

#include <iostream>
#include <string>
#include <utility>

class Address {
public:
    explicit Address(std::string postcode) : postcode_(std::move(postcode)) {}
    std::string label() const { return "Ship to " + postcode_; }
    std::string postcode() const { return postcode_; }

private:
    std::string postcode_;
};

class Customer {
public:
    explicit Customer(Address address) : address_(std::move(address)) {}
    std::string shipping_label() const { return address_.label(); }
    std::string shipping_postcode() const { return address_.postcode(); }

private:
    Address address_;
};

class Order {
public:
    explicit Order(Customer customer) : customer_(std::move(customer)) {}
    std::string shipping_label() const { return customer_.shipping_label(); }
    std::string shipping_postcode() const { return customer_.shipping_postcode(); }

private:
    Customer customer_;
};

void demonstrate_drawback() {
    const Order order(Customer(Address("10115")));
    check(order.shipping_postcode() == "10115", "New data request reaches the owned address through forwarding");
    check(order.shipping_label() == "Ship to 10115", "Original label operation still works");

    // Drawback: exposing ONE more value added methods to Address, Customer, AND
    // Order. Repeating that for dozens of fields can turn the outer classes into
    // large forwarding lists. Prefer meaningful operations such as shipping_label;
    // a deliberate read-only data view may fit a data-heavy screen better.
    std::cout << "Drawback: one postcode request needs three methods to forward "
                 "the value through Order, Customer, and Address.\n";
}

int main() {
    const Order order(Customer(Address("10115")));
    check(order.shipping_label() == "Ship to 10115", "Client does not navigate internal relationships");
    std::cout << order.shipping_label() << '\n';
    demonstrate_drawback();
}