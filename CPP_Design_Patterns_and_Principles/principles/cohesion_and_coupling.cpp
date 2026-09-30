#include "support/check.hpp"

#include <iostream>
#include <stdexcept>

class Stock {
public:
    explicit Stock(int count) : count_(count) {
        if (count < 0) { throw std::invalid_argument("Negative stock"); }
    }
    bool reserve(int quantity) {
        if (quantity <= 0) { throw std::invalid_argument("Positive quantity required"); }
        if (quantity > count_) { return false; }
        count_ -= quantity;
        return true;
    }
    int remaining() const { return count_; }

private:
    int count_;
};

class Fulfillment {
public:
    explicit Fulfillment(Stock& stock) : stock_(stock) {}
    bool place_order(int quantity) { return stock_.reserve(quantity); }

private:
    Stock& stock_;
};

// Deliberately unnecessary layers: neither adds a rule, translation, or boundary.
class StockForwarder {
public:
    explicit StockForwarder(Stock& stock) : stock_(stock) {}
    bool reserve(int quantity) { return stock_.reserve(quantity); }
private:
    Stock& stock_;
};

class ReservationForwarder {
public:
    explicit ReservationForwarder(StockForwarder& stock) : stock_(stock) {}
    bool reserve(int quantity) { return stock_.reserve(quantity); }
private:
    StockForwarder& stock_;
};

void demonstrate_drawback() {
    Stock layered_stock(3);
    StockForwarder first(layered_stock);
    ReservationForwarder second(first);
    Stock direct_stock(3);
    check(second.reserve(2) && direct_stock.reserve(2), "Both paths perform the same reservation");
    check(layered_stock.remaining() == direct_stock.remaining(), "Extra forwarding changes no rule");

    // Drawback: trying to hide every dependency creates more objects to wire and
    // more code to follow. A direct dependency on Stock's clear reserve() contract
    // is reasonable; these two forwarding layers remove no meaningful coupling.
    std::cout << "Drawback: two forwarding layers still depend on Stock and "
                 "produce the same remaining count of 1.\n";
}

int main() {
    Stock stock(3);
    Fulfillment fulfillment(stock);
    check(fulfillment.place_order(2) && stock.remaining() == 1, "Related stock operations stay together");
    check(!fulfillment.place_order(2) && stock.remaining() == 1, "Caller cannot force negative inventory");
    std::cout << "Remaining: " << stock.remaining() << '\n';
    demonstrate_drawback();
}