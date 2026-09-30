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

int main() {
    Stock stock(3);
    Fulfillment fulfillment(stock);
    check(fulfillment.place_order(2) && stock.remaining() == 1, "Related stock operations stay together");
    check(!fulfillment.place_order(2) && stock.remaining() == 1, "Caller cannot force negative inventory");
    std::cout << "Remaining: " << stock.remaining() << '\n';
}