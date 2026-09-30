#include "support/check.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

struct Beverage {
    virtual ~Beverage() = default;
    virtual int cost() const = 0;
    virtual std::string description() const = 0;
};

struct Coffee final : Beverage {
    int cost() const override { return 200; }
    std::string description() const override { return "coffee"; }
};

class BeverageDecorator : public Beverage {
public:
    explicit BeverageDecorator(std::unique_ptr<Beverage> inner) : inner_(std::move(inner)) {
        if (!inner_) { throw std::invalid_argument("Missing beverage"); }
    }

protected:
    std::unique_ptr<Beverage> inner_;
};

class Milk final : public BeverageDecorator {
public:
    using BeverageDecorator::BeverageDecorator;
    int cost() const override { return inner_->cost() + 50; }
    std::string description() const override { return inner_->description() + ", milk"; }
};

class Cinnamon final : public BeverageDecorator {
public:
    using BeverageDecorator::BeverageDecorator;
    int cost() const override { return inner_->cost() + 20; }
    std::string description() const override { return inner_->description() + ", cinnamon"; }
};

int main() {
    std::unique_ptr<Beverage> drink = std::make_unique<Coffee>();
    check(drink->cost() == 200, "Base price in cents");
    drink = std::make_unique<Milk>(std::move(drink));
    drink = std::make_unique<Cinnamon>(std::move(drink));
    check(drink->cost() == 270, "Both decorators must contribute");
    check(drink->description() == "coffee, milk, cinnamon", "Wrapper order");
    std::cout << drink->description() << ": " << drink->cost() << " cents\n";
}