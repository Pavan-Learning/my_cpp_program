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

int main() {
    const Circle original(5);
    auto copy = original.clone();
    check(copy->describe() == original.describe(), "Clone starts with equivalent state");
    copy->set_color("blue");
    check(original.describe() == "red circle r=5", "Clone mutation must not change prototype");
    check(copy->describe() == "blue circle r=5", "Clone preserves derived data");
    std::cout << original.describe() << '\n' << copy->describe() << '\n';
}