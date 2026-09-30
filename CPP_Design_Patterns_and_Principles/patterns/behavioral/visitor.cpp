#include "support/check.hpp"

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

class Circle;
class Rectangle;

struct Visitor {
    virtual ~Visitor() = default;
    virtual void visit(const Circle& circle) = 0;
    virtual void visit(const Rectangle& rectangle) = 0;
};

struct Shape {
    virtual ~Shape() = default;
    virtual void accept(Visitor& visitor) const = 0;
};

class Circle final : public Shape {
public:
    explicit Circle(double radius) : radius_(radius) {}
    double radius() const { return radius_; }
    void accept(Visitor& visitor) const override { visitor.visit(*this); }

private:
    double radius_;
};

class Rectangle final : public Shape {
public:
    Rectangle(double width, double height) : width_(width), height_(height) {}
    double width() const { return width_; }
    double height() const { return height_; }
    void accept(Visitor& visitor) const override { visitor.visit(*this); }

private:
    double width_;
    double height_;
};

class Area final : public Visitor {
public:
    void visit(const Circle& circle) override { total_ += pi * circle.radius() * circle.radius(); }
    void visit(const Rectangle& rectangle) override { total_ += rectangle.width() * rectangle.height(); }
    double total() const { return total_; }
    static constexpr double pi = 3.141592653589793;

private:
    double total_ = 0;
};

class Count final : public Visitor {
public:
    void visit(const Circle&) override { ++count_; }
    void visit(const Rectangle&) override { ++count_; }
    int total() const { return count_; }

private:
    int count_ = 0;
};

int main() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(2));
    shapes.push_back(std::make_unique<Rectangle>(3, 4));
    Area area;
    Count count;
    for (const auto& shape : shapes) {
        shape->accept(area);
        shape->accept(count);
    }
    check(std::abs(area.total() - (4 * Area::pi + 12)) < 1e-9, "Double dispatch selects each shape operation");
    check(count.total() == 2, "Independent operation uses unchanged shapes");
    std::cout << "Visited " << count.total() << " shapes\n";
}