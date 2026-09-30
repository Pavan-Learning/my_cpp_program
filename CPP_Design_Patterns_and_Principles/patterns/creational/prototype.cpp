#include "support/check.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

struct Shape {
    virtual ~Shape() = default;
    virtual std::unique_ptr<Shape> clone() const = 0;
    virtual void set_color(std::string color) = 0;
    virtual std::string describe() const = 0;
};

class Circle final : public Shape {
public:
    explicit Circle(int radius) : radius_(radius) {}
    std::unique_ptr<Shape> clone() const override { return std::make_unique<Circle>(*this); }
    void set_color(std::string color) override { color_ = std::move(color); }
    std::string describe() const override { return color_ + " circle r=" + std::to_string(radius_); }

private:
    int radius_;
    std::string color_ = "red";
};

// Intentionally flawed for an INDEPENDENT-copy requirement: the pointer is
// copied, but the string it owns is shared. Sharing could be correct for a
// different contract; clone() must say which meaning it promises.
struct SharedColorPrototype {
    std::shared_ptr<std::string> color = std::make_shared<std::string>("red");

    std::unique_ptr<SharedColorPrototype> clone() const {
        return std::make_unique<SharedColorPrototype>(*this);
    }
};

void demonstrate_drawback() {
    const SharedColorPrototype original;
    auto shallow_copy = original.clone();
    *shallow_copy->color = "blue";
    check(*original.color == "blue", "Shallow clone also changes the original's shared data");

    // Drawback: a deep copy needs explicit work for owned mutable data. A const
    // outer object also does not make the string behind shared_ptr immutable.
    auto independent_copy = original.clone();
    independent_copy->color = std::make_shared<std::string>(*original.color);
    *independent_copy->color = "green";
    check(*original.color == "blue" && *independent_copy->color == "green",
          "Copying the pointed-to string makes the clone independent");
    std::cout << "Drawback: shallow clone changed original to blue; "
                 "deep clone can change to green independently.\n";
}

int main() {
    const Circle original(5);
    auto copy = original.clone();
    check(copy->describe() == original.describe(), "Clone starts with equivalent state");
    copy->set_color("blue");
    check(original.describe() == "red circle r=5", "Clone mutation must not change prototype");
    check(copy->describe() == "blue circle r=5", "Clone preserves derived data");
    std::cout << original.describe() << '\n' << copy->describe() << '\n';
    demonstrate_drawback();
}