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

struct WidgetFactory {
    virtual ~WidgetFactory() = default;
    virtual std::unique_ptr<Button> button() const = 0;
    virtual std::unique_ptr<Checkbox> checkbox() const = 0;
};

struct LightFactory final : WidgetFactory {
    std::unique_ptr<Button> button() const override { return std::make_unique<LightButton>(); }
    std::unique_ptr<Checkbox> checkbox() const override { return std::make_unique<LightCheckbox>(); }
};

struct DarkFactory final : WidgetFactory {
    std::unique_ptr<Button> button() const override { return std::make_unique<DarkButton>(); }
    std::unique_ptr<Checkbox> checkbox() const override { return std::make_unique<DarkCheckbox>(); }
};

std::string draw_form(const WidgetFactory& factory) {
    return factory.button()->draw() + " + " + factory.checkbox()->draw();
}

int main() {
    check(draw_form(LightFactory{}) == "light button + light checkbox", "Light family");
    check(draw_form(DarkFactory{}) == "dark button + dark checkbox", "Dark family");
    std::cout << draw_form(DarkFactory{}) << '\n';
}