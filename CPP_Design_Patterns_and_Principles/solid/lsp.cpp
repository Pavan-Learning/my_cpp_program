#include "support/check.hpp"

#include <iostream>

namespace before {
class Rectangle {
public:
    virtual ~Rectangle() = default;
    virtual void width(int value) { width_ = value; }
    virtual void height(int value) { height_ = value; }
    int area() const { return width_ * height_; }

protected:
    int width_ = 0;
    int height_ = 0;
};

class Square final : public Rectangle {
public:
    void width(int value) override { width_ = height_ = value; }
    void height(int value) override { width_ = height_ = value; }
};

bool rectangle_contract(Rectangle& rectangle) {
    rectangle.width(4);
    rectangle.height(5);
    return rectangle.area() == 20;
}
}

namespace after {
struct Shape {
    virtual ~Shape() = default;
    virtual int area() const = 0;
};

class Rectangle final : public Shape {
public:
    Rectangle(int width, int height) : width_(width), height_(height) {}
    int area() const override { return width_ * height_; }

private:
    int width_;
    int height_;
};

class Square final : public Shape {
public:
    explicit Square(int side) : side_(side) {}
    int area() const override { return side_ * side_; }

private:
    int side_;
};

int inspect(const Shape& shape) { return shape.area(); }
}

int main() {
    before::Rectangle rectangle;
    before::Square square;
    check(before::rectangle_contract(rectangle), "Base satisfies independent dimensions");
    check(!before::rectangle_contract(square), "Detect the deliberately broken substitution");
    check(after::inspect(after::Rectangle(4, 5)) == 20, "Rectangle obeys read-only shape contract");
    check(after::inspect(after::Square(5)) == 25, "Square obeys the same read-only contract");
    std::cout << "Broken substitution detected; corrected contract verified\n";
}