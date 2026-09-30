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

class HalfPrice final : public BeverageDecorator {
public:
    using BeverageDecorator::BeverageDecorator;
    int cost() const override { return inner_->cost() / 2; }
    std::string description() const override { return inner_->description() + ", half price"; }
};

void demonstrate_drawback() {
    // Drawback: wrappers are applied from the inside out. Discount AFTER milk
    // gives (200 + 50) / 2 = 125; milk AFTER discount gives 200 / 2 + 50 = 150.
    // The same two features can therefore produce different prices.
    const HalfPrice discount_last(std::make_unique<Milk>(std::make_unique<Coffee>()));
    const Milk milk_last(std::make_unique<HalfPrice>(std::make_unique<Coffee>()));
    check(discount_last.cost() == 125 && milk_last.cost() == 150, "Wrapper order changes the result");

    const Milk repeated(std::make_unique<Milk>(std::make_unique<Coffee>()));
    check(repeated.cost() == 300, "The pattern does not reject duplicate features");
    std::cout << "Drawback: discount after milk=125, milk after discount=150, "
                 "and accidental double milk=300 cents.\n";
}

int main() {
    std::unique_ptr<Beverage> drink = std::make_unique<Coffee>();
    check(drink->cost() == 200, "Base price in cents");
    drink = std::make_unique<Milk>(std::move(drink));
    drink = std::make_unique<Cinnamon>(std::move(drink));
    check(drink->cost() == 270, "Both decorators must contribute");
    check(drink->description() == "coffee, milk, cinnamon", "Wrapper order");
    std::cout << drink->description() << ": " << drink->cost() << " cents\n";
    demonstrate_drawback();
}