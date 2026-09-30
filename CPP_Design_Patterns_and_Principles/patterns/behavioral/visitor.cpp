#include "support/check.hpp"

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

class Circle;
class Rectangle;
class Triangle;

struct Visitor {
    virtual ~Visitor() = default;
    virtual void visit(const Circle& circle) = 0;
    virtual void visit(const Rectangle& rectangle) = 0;
    // Drawback: one new shape changes this interface and every existing visitor.
    virtual void visit(const Triangle& triangle) = 0;
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

class Triangle final : public Shape {
public:
    Triangle(double base, double height) : base_(base), height_(height) {}
    double base() const { return base_; }
    double height() const { return height_; }
    void accept(Visitor& visitor) const override { visitor.visit(*this); }

private:
    double base_;
    double height_;
};

class Area final : public Visitor {
public:
    void visit(const Circle& circle) override { total_ += pi * circle.radius() * circle.radius(); }
    void visit(const Rectangle& rectangle) override { total_ += rectangle.width() * rectangle.height(); }
    void visit(const Triangle& triangle) override { total_ += triangle.base() * triangle.height() / 2.0; }
    double total() const { return total_; }
    static constexpr double pi = 3.141592653589793;

private:
    double total_ = 0;
};

class Count final : public Visitor {
public:
    void visit(const Circle&) override { ++count_; }
    void visit(const Rectangle&) override { ++count_; }
    void visit(const Triangle&) override { ++count_; }
    int total() const { return count_; }

private:
    int count_ = 0;
};

void demonstrate_drawback() {
    const Triangle triangle(3, 4);
    Area area;
    Count count;
    triangle.accept(area);
    triangle.accept(count);
    check(area.total() == 6.0 && count.total() == 1, "Both existing visitors must understand Triangle");

    // Area and Count needed new overloads even though they already handled all
    // the old shapes. Omitting either override leaves that visitor abstract,
    // so it cannot be constructed. No intentionally broken code is enabled here.
    std::cout << "Drawback: adding Triangle required changes to Visitor, Area, "
                 "and Count; triangle area=6 and count=1.\n";
}

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
    demonstrate_drawback();
}