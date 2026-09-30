#include "support/check.hpp"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>

class Expiration {
public:
    explicit Expiration(std::function<int()> now) : now_(std::move(now)) {
        if (!now_) { throw std::invalid_argument("Clock required"); }
    }
    bool expired(int deadline) const { return now_() >= deadline; }

private:
    std::function<int()> now_;
};

int main() {
    int clock = 99;
    const Expiration expiration([&clock] { return clock; });
    check(!expiration.expired(100), "Before deadline");
    clock = 100;
    check(expiration.expired(100), "At deadline");
    clock = 101;
    check(expiration.expired(100), "After deadline");
    std::cout << "Time-dependent behavior tested without waiting\n";
}