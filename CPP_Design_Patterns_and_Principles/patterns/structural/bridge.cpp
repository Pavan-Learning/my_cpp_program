#include "support/check.hpp"

#include <iostream>
#include <string>

struct Renderer {
    virtual ~Renderer() = default;
    virtual std::string circle(int radius) const = 0;
    // Drawback: a new drawing operation changes this interface and EVERY tool.
    virtual std::string rectangle(int width, int height) const = 0;
};

struct VectorRenderer final : Renderer {
    std::string circle(int radius) const override { return "vector circle " + std::to_string(radius); }
    std::string rectangle(int width, int height) const override {
        return "vector rectangle " + std::to_string(width) + "x" + std::to_string(height);
    }
};

struct RasterRenderer final : Renderer {
    std::string circle(int radius) const override { return "raster circle " + std::to_string(radius); }
    std::string rectangle(int width, int height) const override {
        return "raster rectangle " + std::to_string(width) + "x" + std::to_string(height);
    }
};

class Shape {
public:
    explicit Shape(const Renderer& renderer) : renderer_(renderer) {}
    virtual ~Shape() = default;
    virtual std::string draw() const = 0;

protected:
    const Renderer& renderer_;
};

class Circle final : public Shape {
public:
    Circle(const Renderer& renderer, int radius) : Shape(renderer), radius_(radius) {}
    std::string draw() const override { return renderer_.circle(radius_); }

private:
    int radius_;
};

class Rectangle final : public Shape {
public:
    Rectangle(const Renderer& renderer, int width, int height)
        : Shape(renderer), width_(width), height_(height) {}
    std::string draw() const override { return renderer_.rectangle(width_, height_); }

private:
    int width_;
    int height_;
};

void demonstrate_drawback() {
    // Bridge separates shape selection from tool selection, but not every future
    // feature is independent. A circle-only Renderer could not draw rectangles.
    // Supporting this example required editing BOTH existing renderer classes.
    const VectorRenderer vector;
    const RasterRenderer raster;
    check(Rectangle(vector, 3, 4).draw() == "vector rectangle 3x4", "Vector tool needs the new operation");
    check(Rectangle(raster, 3, 4).draw() == "raster rectangle 3x4", "Raster tool needs it too");
    std::cout << "Drawback: adding rectangle() changes Renderer, VectorRenderer, "
                 "and RasterRenderer, not just the new Rectangle class.\n";
}

int main() {
    const VectorRenderer vector;
    const RasterRenderer raster;
    const Circle scalable(vector, 4);
    const Circle pixels(raster, 4);
    check(scalable.draw() == "vector circle 4", "Vector implementation");
    check(pixels.draw() == "raster circle 4", "Raster implementation");
    std::cout << scalable.draw() << '\n' << pixels.draw() << '\n';
    demonstrate_drawback();
}