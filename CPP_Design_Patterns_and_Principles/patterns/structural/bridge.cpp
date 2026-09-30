#include "support/check.hpp"

#include <iostream>
#include <string>

struct Renderer {
    virtual ~Renderer() = default;
    virtual std::string circle(int radius) const = 0;
};

struct VectorRenderer final : Renderer {
    std::string circle(int radius) const override { return "vector circle " + std::to_string(radius); }
};

struct RasterRenderer final : Renderer {
    std::string circle(int radius) const override { return "raster circle " + std::to_string(radius); }
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

int main() {
    const VectorRenderer vector;
    const RasterRenderer raster;
    const Circle scalable(vector, 4);
    const Circle pixels(raster, 4);
    check(scalable.draw() == "vector circle 4", "Vector implementation");
    check(pixels.draw() == "raster circle 4", "Raster implementation");
    std::cout << scalable.draw() << '\n' << pixels.draw() << '\n';
}