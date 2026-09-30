#include "support/check.hpp"

#include <functional>
#include <iostream>
#include <memory>
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

void demonstrate_drawback() {
    std::function<int()> borrowed_clock;
    {
        auto clock = std::make_shared<int>(99);
        const std::weak_ptr<int> observed = clock;
        borrowed_clock = [observed] {
            const auto alive = observed.lock();
            if (!alive) { throw std::runtime_error("Injected clock no longer exists"); }
            return *alive;
        };
        check(!Expiration(borrowed_clock).expired(100), "Dependency works while its owner is alive");
    }
    const Expiration expired_dependency(borrowed_clock);
    bool rejected = false;
    try { static_cast<void>(expired_dependency.expired(100)); }
    catch (const std::runtime_error&) { rejected = true; }
    check(rejected, "Owning the callback does not own the clock it weakly refers to");

    // Drawback: injection moves lifetime decisions to setup. weak_ptr lets this
    // example detect expiration safely; a dangling raw reference must NOT be read.
    // Capturing a shared owner is one option when extending lifetime is intended.
    const Expiration owned_clock([clock = std::make_shared<int>(100)] { return *clock; });
    check(owned_clock.expired(100), "Owning capture keeps its clock alive");
    std::cout << "Drawback: an injected callback can outlive its dependency; "
                 "weak capture detects expiration, owning capture preserves lifetime.\n";
}

int main() {
    int clock = 99;
    const Expiration expiration([&clock] { return clock; });
    check(!expiration.expired(100), "Before deadline");
    clock = 100;
    check(expiration.expired(100), "At deadline");
    clock = 101;
    check(expiration.expired(100), "After deadline");
    std::cout << "Time-dependent behavior tested without waiting\n";
    demonstrate_drawback();
}