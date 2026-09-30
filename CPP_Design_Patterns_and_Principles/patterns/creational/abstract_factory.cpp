#include "support/check.hpp"

#include <iostream>
#include <memory>
#include <string>

struct Button {
    virtual ~Button() = default;
    virtual std::string draw() const = 0;
};

struct Checkbox {
    virtual ~Checkbox() = default;
    virtual std::string draw() const = 0;
};

struct LightButton final : Button {
    std::string draw() const override { return "light button"; }
};

struct DarkButton final : Button {
    std::string draw() const override { return "dark button"; }
};

struct LightCheckbox final : Checkbox {
    std::string draw() const override { return "light checkbox"; }
};

struct DarkCheckbox final : Checkbox {
    std::string draw() const override { return "dark checkbox"; }
};

// Drawback: adding a CATEGORY needs a new product type and support in EVERY
// factory. Compare these slider additions with adding just one new theme.
struct Slider {
    virtual ~Slider() = default;
    virtual std::string draw() const = 0;
};

struct LightSlider final : Slider {
    std::string draw() const override { return "light slider"; }
};

struct DarkSlider final : Slider {
    std::string draw() const override { return "dark slider"; }
};

struct WidgetFactory {
    virtual ~WidgetFactory() = default;
    virtual std::unique_ptr<Button> button() const = 0;
    virtual std::unique_ptr<Checkbox> checkbox() const = 0;
    virtual std::unique_ptr<Slider> slider() const = 0;
};

struct LightFactory final : WidgetFactory {
    std::unique_ptr<Button> button() const override { return std::make_unique<LightButton>(); }
    std::unique_ptr<Checkbox> checkbox() const override { return std::make_unique<LightCheckbox>(); }
    std::unique_ptr<Slider> slider() const override { return std::make_unique<LightSlider>(); }
};

struct DarkFactory final : WidgetFactory {
    std::unique_ptr<Button> button() const override { return std::make_unique<DarkButton>(); }
    std::unique_ptr<Checkbox> checkbox() const override { return std::make_unique<DarkCheckbox>(); }
    std::unique_ptr<Slider> slider() const override { return std::make_unique<DarkSlider>(); }
};

std::string draw_form(const WidgetFactory& factory) {
    return factory.button()->draw() + " + " + factory.checkbox()->draw();
}

void demonstrate_drawback() {
    const LightFactory light;
    const DarkFactory dark;
    check(light.slider()->draw() == "light slider", "Light factory must support the new category");
    check(dark.slider()->draw() == "dark slider", "Dark factory must also support the new category");

    // A factory helps us choose a matching family; these return types do not
    // forbid taking one product from EACH factory. That needs a stronger design.
    const auto mixed = light.button()->draw() + " + " + dark.checkbox()->draw();
    check(mixed == "light button + dark checkbox", "Mixing families is still possible");
    std::cout << "Drawback: Slider changes every factory; mixed products are still allowed: "
              << mixed << '\n';
}

int main() {
    check(draw_form(LightFactory{}) == "light button + light checkbox", "Light family");
    check(draw_form(DarkFactory{}) == "dark button + dark checkbox", "Dark family");
    std::cout << draw_form(DarkFactory{}) << '\n';
    demonstrate_drawback();
}