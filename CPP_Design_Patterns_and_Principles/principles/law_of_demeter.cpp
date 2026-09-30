#include "support/check.hpp"

#include <iostream>
#include <string>
#include <utility>

class Address {
public:
    explicit Address(std::string postcode) : postcode_(std::move(postcode)) {}
    std::string label() const { return "Ship to " + postcode_; }

private:
    std::string postcode_;
};

class Customer {
public:
    explicit Customer(Address address) : address_(std::move(address)) {}
    std::string shipping_label() const { return address_.label(); }

private:
    Address address_;
};

class Order {
public:
    explicit Order(Customer customer) : customer_(std::move(customer)) {}
    std::string shipping_label() const { return customer_.shipping_label(); }

private:
    Customer customer_;
};

int main() {
    const Order order(Customer(Address("10115")));
    check(order.shipping_label() == "Ship to 10115", "Client does not navigate internal relationships");
    std::cout << order.shipping_label() << '\n';
}